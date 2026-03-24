/*
 * Adds late move reductions (LMR): quiet moves ordered late at the same node are
 * searched with reduced depth first.
 *
 * If the reduced search fails high or produces a cutoff, we may re-search at full
 * depth. Rationale: in many positions early moves contain the best reply; later
 * moves are statistically weaker, so less depth saves nodes while preserving
 * correctness via re-search when the scout result is suspicious.
 */
#include "castro.h"
#include "castro_additions.h"
#include "eval.h"
#include "search.h"
#include "uci.h"
#include "moveordering.h"
#include "tt.h"
#include <stdio.h>

static int negamax_rec(Board* board,
                       EvalFn eval,
                       OrderFn order,
                       int depth,
                       int ply,
                       int alpha,
                       int beta,
                       SearchConfig* config);

Move negamax_id_ab_q_mo_tt_nmp_lmr(Board* board, EvalFn eval, OrderFn order, SearchConfig* config)
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

            int score = -negamax_rec(board, eval, order, currentDepth - 1, 1, -beta, -alpha, config);
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

static int negamax_rec(Board* board, EvalFn evaluate, OrderFn order, int depth, int ply, int alpha, int beta, SearchConfig* config)
{
    // 1. Periodic timer check & TT Probe
    if ((g_searchStats.nodes & 2047) == 0) {
        search_time_up(); 
    }
    if (search_should_stop()) return 0;

    if (depth <= 0) {
        return quiescence(board, alpha, beta, ply, evaluate, order);
    }

    int tt_score = 0;
    Move tt_move = NULL_MOVE;
    if (tt_probe(board->hash, depth, alpha, beta, ply, &tt_score, &tt_move)) {
        return tt_score;
    }

    g_searchStats.nodes++;

    // 2. Null Move Pruning (NMP)
    if (depth >= 3 && !castro_IsInCheck(board) && castro_HasNonPawnMaterial(board, board->turn)) {
        castro_MakeNullMove(board);
        // Standard R=3 reduction for NMP
        int nullScore = -negamax_rec(board, evaluate, order, depth - 1 - 3, ply + 1, -beta, -beta + 1, config);
        castro_UnmakeNullMove(board);

        if (nullScore >= beta) {
            if (nullScore > MATE_SCORE) nullScore = beta;
            return nullScore; 
        }
    }

    // 3. Move Generation and Ordering
    Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);
    order(board, moves.list, moves.count, ply, tt_move);

    int original_alpha = alpha;
    int max_eval = -INF;
    Move best_move_found = NULL_MOVE;
    int legal_moves_count = 0;
    const bool parent_in_check = castro_IsInCheck(board);

    for (size_t i = 0; i < moves.count; i++) {
        Move move = moves.list[i];
        bool is_capture = castro_IsCapture(board, move);

        if (!castro_MakeMove(board, move)) continue; 
        legal_moves_count++;
        
        // --- Defined missing variables for LMR ---
        int move_index = legal_moves_count - 1; // 0-based index of legal moves
        int next_depth = depth - 1;
        bool gives_check = castro_IsInCheck(board);
        int score;

        // 4. Late Move Reductions (LMR)
        // Only reduce quiet moves (not captures/checks) searched late in the list
        bool use_lmr = config->useLMR && depth >= 3 && move_index >= 4 
                       && !parent_in_check && !is_capture && !gives_check;

        if (use_lmr) {
            int R = 1; // Base reduction
            if (depth >= 6 && move_index >= 6) R++;
            if (depth >= 10 && move_index >= 12) R++; // Deeper reduction for very late moves

            int reduced_depth = next_depth - R;
            if (reduced_depth < 0) reduced_depth = 0;

            // Search with a reduced depth and a null window
            score = -negamax_rec(board, evaluate, order, reduced_depth, ply + 1, -(alpha + 1), -alpha, config);

            // Re-search: If the reduced search suggests the move is actually good, search again at full depth
            if (score > alpha) {
                score = -negamax_rec(board, evaluate, order, next_depth, ply + 1, -beta, -alpha, config);
            }
        } else {
            // Standard Full Depth Search
            score = -negamax_rec(board, evaluate, order, next_depth, ply + 1, -beta, -alpha, config);
        }

        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        if (score > max_eval) {
            max_eval = score;
            best_move_found = move;
        }
        if (score > alpha) alpha = score;
        if (alpha >= beta) break; // Beta cutoff
    }

    // 5. Terminal check (Mate/Stalemate)
    if (legal_moves_count == 0) {
        if (castro_IsInCheck(board)) return -INF + ply;
        return 0;
    }

    // 6. TT Store
    TTNodeType type = TT_EXACT;
    if (max_eval <= original_alpha) type = TT_UPPERBOUND;
    else if (max_eval >= beta)      type = TT_LOWERBOUND;

    tt_store(board->hash, depth, max_eval, type, best_move_found, ply);
    return max_eval;
}
