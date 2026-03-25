// Search Implemenation:
// Negamax
//   + Iterative Deepening
//   + Alpha-Beta Pruning
//   + Quiescence
//   + Move Ordering
//   + Transposition Table

#include "search.h"
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
    printf("info using new search with three-fold repetition check\n");
    Move best_move = NULL_MOVE;
    g_searchStats.nodes = 0;

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
            uci_report_search(currentDepth, bestScore, g_searchStats.nodes, search_elapsed_ms(), moveBuf);
            
            // If we found a mate, no need to search deeper
            if (bestScore > (INF - MAX_PLY)) break; 
        } else {
            break; 
        }
    }

    return best_move;
}

static int negamax(Board* board, EvalFn eval, OrderFn order, int depth, int ply, int a, int b, SearchConfig* config)
{
    if (ply > 0 && castro_IsThreefoldRepetition(board)) {
        return 0;
    } 

    g_searchStats.nodes++;
    int original_alpha = a;

    Move tt_move = NULL_MOVE;
    int tt_score = 0;
    if (tt_probe(board->hash, depth, a, b, ply, &tt_score, &tt_move)) {
        return tt_score;
    }

    if (depth <= 0) return quiescence(board, a, b, ply, eval, order);

    Moves legal = castro_GenerateLegalMoves(board);
    if (legal.count == 0) {
        if (castro_IsInCheck(board)) return -INF + ply;
        return 0;
    }

    order(board, legal.list, legal.count, ply, tt_move);

    Move bestMove = NULL_MOVE;
    int bestScore = -INF;

    for (size_t i = 0; i < legal.count; ++i) {
        if (!castro_MakeMove(board, legal.list[i])) continue;
        
        int score = -negamax(board, eval, order, depth - 1, ply + 1, -b, -a, config);
        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        if (score > bestScore) {
            bestScore = score;
            bestMove = legal.list[i];
        }

        if (score > a) a = score;
        if (a >= b) break; // Cutoff
    }

    TTNodeType type = (bestScore <= original_alpha) ? TT_UPPERBOUND : 
                      (bestScore >= b) ? TT_LOWERBOUND : TT_EXACT;
    
    tt_store(board->hash, depth, bestScore, type, bestMove, ply);

    return bestScore;
}
