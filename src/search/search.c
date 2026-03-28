// Search Implemenation:
// Negamax
//   + Iterative Deepening
//   + Alpha-Beta Pruning
//   + Quiescence
//   + Move Ordering
//   + Transposition Table
//   + Null Move Pruning
//   + Late Move Reductions
//   + Syzygy

#include "search.h"
#include "castro.h"
#include "syzygy.h"
#include "tt.h"
#include "uci.h"

static int negamax(
    Board* board,
    EvalFn eval,
    OrderFn order,
    int depth, int ply,
    int a, int b,
    SearchConfig* config
);

Move search(Board* board, EvalFn eval, OrderFn order, SearchConfig* config)
{
    tt_inc_generation();

    Move tb_move = NULL_MOVE;
    size_t piece_count = castro_PieceCount(board);
    if (config->useSyzygy && piece_count <= config->syzygyProbeLimit) {
        if (syzygy_probe_root(board, config->syzygy50MoveRule, &tb_move)) {
            printf("info string tablebase hit\n");
            return tb_move; 
        }
    }

    Move best_move = NULL_MOVE;
    g_searchStats.nodes = 0;
    g_searchStats.qnodes = 0;
    g_searchStats.selDepth = 0;

    search_start_timer(config->timeLimitMs);

    // Iterative Deepening
    for (int currentDepth = 1; currentDepth <= config->maxDepth; currentDepth++) {
        if (search_time_up()) break;

        Moves legal = castro_GenerateLegalMoves(board);
        order(board, legal.list, legal.count, 0, best_move);

        int alpha = -INF;
        int beta = INF;
        int bestScore = -INF;
        Move currentBestMove = NULL_MOVE;

        for (size_t i = 0; i < legal.count; i++) {
            if (!castro_MakeMove(board, legal.list[i])) continue;

            int score = -negamax(board, eval, order, currentDepth - 1, 1, -beta, -alpha, config);
            castro_UnmakeMove(board);

            // If time ran out, discard results from this depth to avoid playing a blunder
            if (search_should_stop()) break;

            if (score > bestScore) {
                bestScore = score;
                currentBestMove = legal.list[i];
            }
            if (score > alpha) alpha = score;
        }

        // Only update the global best_move if we completed the depth (or at least the first move)
        if (!search_should_stop()) {
            best_move = currentBestMove;
            
            char moveBuf[10];
            castro_MoveToString(best_move, moveBuf);
            uci_report_search(currentDepth, bestScore, search_elapsed_ms(), moveBuf);
            
            // If we found a mate, no need to search deeper
            if (bestScore > (INF - MAX_PLY)) break; 
        } else {
            break; 
        }
    }

    return best_move;
}

static int negamax(Board* board, EvalFn eval, OrderFn order, int depth, int ply, int alpha, int beta, SearchConfig* config)
{
    // 1. Static Checks (Repetition / 50-move rule)
    if (ply > 0 && (castro_IsThreefoldRepetition(board) || board->halfmove >= 100)) {
        return 0;
    }

    // 2. TT Probe: Fastest check, do this first.
    Move tt_move = NULL_MOVE;
    int tt_score = 0;
    if (config->useTT && tt_probe(board->hash, depth, alpha, beta, ply, &tt_score, &tt_move)) {
        return tt_score;
    }

    int mated_score = -INF + ply;
    if (alpha < mated_score) alpha = mated_score;
    if (beta <= alpha) return alpha;

    if ((g_searchStats.nodes & 2047) == 0) {
        if (search_time_up()) {
            uci_state.stopRequested = true;
        }
    }
    if (search_should_stop()) return 0;

    g_searchStats.nodes++;
    if (ply > g_searchStats.selDepth)
        g_searchStats.selDepth = ply;

    // 3. Syzygy Probe: Only if TT didn't give us a result.
    int piece_count = castro_PieceCount(board);
    if (config->useSyzygy && ply > 0 && piece_count <= config->syzygyProbeLimit) {
        int tb_score = syzygy_probe_wdl(board, config->syzygy50MoveRule);
        if (tb_score != SYZYGY_PROBE_FAILED) {
            if (config->useTT)
                tt_store(board->hash, depth, tb_score, TT_EXACT, NULL_MOVE, ply);
            g_searchStats.tbHits++;
            return tb_score;
        }
    }

    // 4. Base Case: Leaf Node
    if (depth <= 0) return config->useQuiescence ? quiescence(board, alpha, beta, ply, eval, order) : 0;

    // 5. Null Move Pruning
    if (config->useNMP && depth >= 3 && !castro_IsInCheck(board) && castro_HasNonPawnMaterial(board, board->turn)) {
        castro_MakeNullMove(board);
        int score = -negamax(board, eval, order, depth - 1 - 3, ply + 1, -beta, -beta + 1, config);
        castro_UnmakeNullMove(board);

        if (score >= beta) {
            // Prune if the null move search fails high
            return (score >= MATE_SCORE) ? beta : score; 
        }
    }

    // 6. Move Generation and Search
    int original_alpha = alpha;
    Moves legal = castro_GenerateLegalMoves(board);
    if (legal.count == 0) {
        if (castro_IsInCheck(board)) return -INF + ply;
        return 0;
    }

    order(board, legal.list, legal.count, ply, tt_move);

    Move bestMove = NULL_MOVE;
    int bestScore = -INF;
    const bool parent_in_check = castro_IsInCheck(board);

    for (size_t i = 0; i < legal.count; ++i) {
        bool is_capture = castro_IsCapture(board, legal.list[i]);
        if (!castro_MakeMove(board, legal.list[i])) continue;
        bool gives_check = castro_IsInCheck(board);
        
        // Only reduce quiet moves (not captures/checks) searched late in the list
        bool use_lmr = config->useLMR && depth >= 3 && i >= 4 
                       && !parent_in_check && !is_capture && !gives_check;

        int score = 0;
        if (use_lmr) {
            int R = 1 + (depth / 6) + (i / 10); // Standard linear-ish reduction
            int reduced_depth = depth - 1 - R;
            if (reduced_depth < 1) reduced_depth = 1; // Don't reduce into Quiescence

            // 1. Reduced Depth, Null Window Search
            score = -negamax(board, eval, order, reduced_depth, ply + 1, -(alpha + 1), -alpha, config);

            // 2. If it still looks promising, re-search at Full Depth, Null Window
            if (score > alpha && reduced_depth < depth - 1) {
                score = -negamax(board, eval, order, depth - 1, ply + 1, -(alpha + 1), -alpha, config);
            }

            // 3. If it's STILL better than alpha, finally do a Full Depth, Full Window search
            if (score > alpha) {
                score = -negamax(board, eval, order, depth - 1, ply + 1, -beta, -alpha, config);
            }
        } else {
            // Standard Full Depth Search
            score = -negamax(board, eval, order, depth - 1, ply + 1, -beta, -alpha, config);
        }

        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        if (score > bestScore) {
            bestScore = score;
            bestMove = legal.list[i];
        }

        if (score > alpha) alpha = score;
        if (alpha >= beta) break; 
    }

    // 7. Store Result
    if (config->useTT) {
        TTNodeType type = (bestScore <= original_alpha) ? TT_UPPERBOUND : 
            (bestScore >= beta) ? TT_LOWERBOUND : TT_EXACT;

        tt_store(board->hash, depth, bestScore, type, bestMove, ply);
    }

    return bestScore;
}
