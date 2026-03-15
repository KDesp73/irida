#include "castro.h"
#include "eval.h"
#include "search.h"
#include "uci.h"
#include "moveordering.h"
#include <limits.h>
#include <stdio.h>

static int negamax_rec(Board* board, EvalFn eval, OrderFn order, int depth, int ply, int alpha, int beta);

Move negamax_id_ab_q_mo(Board* board, EvalFn eval, OrderFn order, SearchConfig* config) 
{
    Move best_move = NULL_MOVE;
    g_searchStats.nodes = 0;
    g_searchStats.qnodes = 0;

    search_start_timer(config->timeLimitMs);

    for (int currentDepth = 1; currentDepth <= config->maxDepth; currentDepth++) {
        
        if (search_time_up()) break;

        Moves legal = castro_GenerateMoves(board, MOVE_LEGAL);
        
        // Use the order function with ply 0 at the root
        order(board, legal.list, legal.count, 0);

        int alpha = -INF;
        int beta = INF;
        int bestScore = -INF;
        Move currentBestMove = NULL_MOVE;

        for (size_t i = 0; i < legal.count; i++) {
            castro_MakeMove(board, legal.list[i]);
            
            // Initial call to recursion: depth is currentDepth-1, ply is 1
            int score = -negamax_rec(board, eval, order, currentDepth - 1, 1, -beta, -alpha);
            
            castro_UnmakeMove(board);

            if (search_should_stop()) break;

            if (score > bestScore) {
                bestScore = score;
                currentBestMove = legal.list[i];
            }
            if (score > alpha) alpha = score;
        }

        // Only update best_move if the search at this depth completed fully
        if (!search_should_stop()) {
            best_move = currentBestMove;
            char moveBuf[10];
            castro_MoveToString(best_move, moveBuf);
            
            // Report progress to the GUI
            uci_report_search(currentDepth, bestScore, g_searchStats.nodes + g_searchStats.qnodes, search_elapsed_ms(), moveBuf);
            
            // Exit if we found a forced checkmate
            if (bestScore > (INF - MAX_PLY)) break; 
        } else {
            break; 
        }
    }

    return best_move;
}

/* * Recursive Negamax with Alpha-Beta Pruning
 */
static int negamax_rec(Board* board, EvalFn eval, OrderFn order, int depth, int ply, int alpha, int beta)
{
    // Periodically check timer every 2048 nodes
    if ((g_searchStats.nodes & 2047) == 0) {
        if (search_time_up()) {
            // Signal search_should_stop() to return true
        }
    }
    if (search_should_stop()) return 0;

    g_searchStats.nodes++;

    // 1. Terminal Check
    Moves legal = castro_GenerateMoves(board, MOVE_LEGAL);
    if (legal.count == 0) {
        if (castro_IsInCheck(board)) {
            // Return mate score relative to current ply
            return -INF + ply; 
        }
        return 0; // Stalemate
    }

    // 2. Base Case: Transition to Quiescence Search
    if (depth <= 0) {
        return quiescence(board, alpha, beta, ply, eval, order);
    }

    order(board, legal.list, legal.count, ply);

    int max_eval = -INF;
    for (size_t i = 0; i < legal.count; i++) {
        castro_MakeMove(board, legal.list[i]);
        int score = -negamax_rec(board, eval, order, depth - 1, ply + 1, -beta, -alpha);
        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        if (score > max_eval) max_eval = score;
        if (score > alpha)    alpha = score;

        // Alpha-Beta Cutoff
        if (alpha >= beta) break; 
    }
    return max_eval;
}

