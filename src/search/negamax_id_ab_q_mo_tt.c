#include "castro.h"
#include "eval.h"
#include "search.h"
#include "uci.h"
#include "moveordering.h"
#include "tt.h"
#include <limits.h>
#include <stdio.h>

static int negamax_rec(Board* board, EvalFn eval, OrderFn order, int depth, int ply, int alpha, int beta);

Move negamax_id_ab_q_mo_tt(Board* board, EvalFn eval, OrderFn order, SearchConfig* config)
{
    Move best_move = NULL_MOVE;
    g_searchStats.nodes = 0;

    search_start_timer(config->timeLimitMs);

    for (int currentDepth = 1; currentDepth <= config->maxDepth; currentDepth++) {

        if (search_time_up()) break;

        Moves legal = castro_GenerateLegalMoves(board);

        // Order using the best move from the previous iteration as the TT hint.
        // On the first iteration best_move is NULL_MOVE, which is fine.
        order(board, legal.list, legal.count, 0, best_move);

        int alpha      = -INF;
        int beta       = INF;
        int bestScore  = -INF;
        Move currentBestMove = NULL_MOVE;

        for (size_t i = 0; i < legal.count; i++) {
            if (!castro_MakeMove(board, legal.list[i])) continue;

            int score = -negamax_rec(board, eval, order, currentDepth - 1, 1, -beta, -alpha);
            castro_UnmakeMove(board);

            // If time ran out during this move, discard the whole depth's results
            // to avoid playing a blunder caused by an incomplete search.
            if (search_should_stop()) break;

            if (score > bestScore) {
                bestScore        = score;
                currentBestMove  = legal.list[i];
            }
            if (score > alpha) alpha = score;
        }

        if (!search_should_stop()) {
            best_move = currentBestMove;

            char moveBuf[10];
            castro_MoveToString(best_move, moveBuf);
            uci_report_search(currentDepth, bestScore, g_searchStats.nodes, search_elapsed_ms(), moveBuf);

            // Found a forced mate; no point searching deeper.
            if (bestScore > (INF - MAX_PLY)) break;
        } else {
            break;
        }
    }

    return best_move;
}

static int negamax_rec(Board* board, EvalFn evaluate, OrderFn order, int depth, int ply, int alpha, int beta)
{
    // -------------------------------------------------------------------
    // 1. Periodic timer check.
    //    Without this, a time limit that expires deep in the tree will not
    //    be noticed until we unwind all the way back to the root, wasting
    //    potentially large amounts of time in slow/deep positions.
    // -------------------------------------------------------------------
    if ((g_searchStats.nodes & 2047) == 0) {
        search_time_up(); // updates the internal should_stop flag
    }
    if (search_should_stop()) return 0;

    // -------------------------------------------------------------------
    // 2. TT Probe.
    //    Must happen AFTER the depth/quiescence branch below so that a
    //    cached result at depth 0 does not bypass quiescence and allow
    //    the horizon effect to bleed through cached scores.
    //    We probe here (before the terminal check) because a TT hit saves
    //    us from generating moves at all for non-zero depths.
    // -------------------------------------------------------------------
    int  tt_score = 0;
    Move tt_move  = NULL_MOVE;

    // -------------------------------------------------------------------
    // 3. Quiescence transition.
    //    Must be checked BEFORE the TT probe so that depth-0 nodes always
    //    enter quiescence search rather than potentially returning a
    //    shallow cached score that ignores tactical continuations.
    // -------------------------------------------------------------------
    if (depth <= 0) {
        return quiescence(board, alpha, beta, ply, evaluate, order);
    }

    // Now it is safe to probe: we are at depth >= 1, so a cached score
    // was produced by a proper minimax search, not skipped quiescence.
    if (tt_probe(board->hash, depth, alpha, beta, ply, &tt_score, &tt_move)) {
        return tt_score;
    }

    // -------------------------------------------------------------------
    // 4. Move generation, ordering, and search.
    //    We use pseudo-legal generation and validate with MakeMove so that
    //    the legality test is free (we must call MakeMove anyway).  The
    //    terminal check (checkmate / stalemate) is derived from whether any
    //    pseudo-legal move turned out to be legal.
    // -------------------------------------------------------------------
    g_searchStats.nodes++;

    Moves moves = castro_GenerateMoves(board, MOVE_PSEUDO);
    order(board, moves.list, moves.count, ply, tt_move);

    int  original_alpha    = alpha;
    int  max_eval          = -INF;
    Move best_move_found   = NULL_MOVE;
    int  legal_moves_count = 0;

    for (size_t i = 0; i < moves.count; i++) {
        if (!castro_MakeMove(board, moves.list[i])) continue; // illegal: skip
        legal_moves_count++;

        int score = -negamax_rec(board, evaluate, order, depth - 1, ply + 1, -beta, -alpha);
        castro_UnmakeMove(board);

        // Do not store partial results in the TT if time expired.
        if (search_should_stop()) return 0;

        if (score > max_eval) {
            max_eval        = score;
            best_move_found = moves.list[i];
        }
        if (score > alpha) alpha = score;
        if (alpha >= beta) break; // Beta cutoff
    }

    // -------------------------------------------------------------------
    // 5. Terminal position detection.
    //    If no pseudo-legal move was legal the position is terminal.
    //    Checkmate scores are depth-to-mate adjusted so the engine prefers
    //    shorter mates (lower ply → less negative → preferred when negated).
    // -------------------------------------------------------------------
    if (legal_moves_count == 0) {
        if (castro_IsInCheck(board))
            return -INF + ply; // Checkmate
        return 0;              // Stalemate
    }

    // -------------------------------------------------------------------
    // 6. TT Storage.
    //    Classify the node type so future probes can make the right call:
    //      EXACT      – score is within [original_alpha, beta)
    //      UPPERBOUND – score failed low (all moves were bad for us)
    //      LOWERBOUND – score failed high (caused a beta cutoff)
    // -------------------------------------------------------------------
    TTNodeType type = TT_EXACT;
    if      (max_eval <= original_alpha) type = TT_UPPERBOUND;
    else if (max_eval >= beta)           type = TT_LOWERBOUND;

    tt_store(board->hash, depth, max_eval, type, best_move_found, ply);

    return max_eval;
}
