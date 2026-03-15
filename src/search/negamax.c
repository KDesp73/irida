#include "castro.h"
#include "eval.h"
#include "search.h"
#include "uci.h"
#include "utils.h"
#include <limits.h>

static int negamax_rec(Board* board, EvalFn evaluate, int depth, int alpha, int beta);

Move negamax(Board* board, EvalFn eval, OrderFn order, SearchConfig* config) 
{
    UNUSED(order);
    Move best_move = NULL_MOVE;
    g_searchStats.nodes = 0;

    search_start_timer(config->timeLimitMs);

    for (int currentDepth = 1; currentDepth <= config->maxDepth; currentDepth++) {
        
        if (search_time_up()) break;

        Moves legal = castro_GenerateLegalMoves(board);
        
        // TODO: Move ordering here

        int alpha = -INF;
        int beta = INF;
        int bestScore = -INF;
        Move currentBestMove = NULL_MOVE;

        for (size_t i = 0; i < legal.count; i++) {
            castro_MakeMove(board, legal.list[i]);
            int score = -negamax_rec(board, eval, currentDepth - 1, -beta, -alpha);
            castro_UnmakeMove(board);

            // If time ran out during the recursive call, discard this depth
            if (search_should_stop()) break;

            if (score > bestScore) {
                bestScore = score;
                currentBestMove = legal.list[i];
            }
            if (score > alpha) alpha = score;
        }

        if (!search_should_stop()) {
            best_move = currentBestMove;
            char moveBuf[10];
            castro_MoveToString(best_move, moveBuf);
            uci_report_search(currentDepth, bestScore, g_searchStats.nodes, search_elapsed_ms(), moveBuf);
            
            if (bestScore > (INF - MAX_PLY)) break; 
        } else {
            break; 
        }
    }

    return best_move;
}

static int negamax_rec(Board* board, EvalFn evaluate, int depth, int alpha, int beta)
{
    if (search_should_stop()) return 0;

    g_searchStats.nodes++;

    // 1. Terminal Check
    Moves legal = castro_GenerateLegalMoves(board);
    if (legal.count == 0) {
        if (castro_IsInCheck(board)) return -INF + (MAX_PLY - depth); 
        return 0;
    }

    // 2. Base Case
    if (depth <= 0) {
        return evaluate(board); 
    }

    int max_eval = -INF;

    for (size_t i = 0; i < legal.count; i++) {
        castro_MakeMove(board, legal.list[i]);
        int score = -negamax_rec(board, evaluate, depth - 1, -beta, -alpha);
        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        if (score > max_eval) max_eval = score;
        if (score > alpha)    alpha = score;

        if (alpha >= beta) break; 
    }
    return max_eval;
}
