#include "castro.h"
#include "eval.h"
#include "search.h"
#include "uci.h"
#include <limits.h>

static int negamax_rec(Board* board, EvalFn evaluate, OrderFn order, int depth, int ply, int alpha, int beta);

Move negamax_id_ab_q_mo(Board* board, EvalFn eval, OrderFn order, SearchConfig* config) 
{
    Move best_move = NULL_MOVE;
    g_searchStats.nodes = 0;

    search_start_timer(config->timeLimitMs);

    // Iterative Deepening
    for (int currentDepth = 1; currentDepth <= config->maxDepth; currentDepth++) {
        
        if (search_time_up()) break;

        // 1. Generate moves once at the root
        Moves legal = castro_GenerateLegalMoves(board);

        order(board, legal.list, legal.count, 0, best_move);

        int alpha = -INF;
        int beta = INF;
        int bestScore = -INF;
        Move currentBestMove = NULL_MOVE;

        for (size_t i = 0; i < legal.count; i++) {
            if (!castro_MakeMove(board, legal.list[i])) continue;

            // 3. Pass ply = 1 to the first recursive call
            int score = -negamax_rec(board, eval, order, currentDepth - 1, 1, -beta, -alpha);
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

static int negamax_rec(Board* board, EvalFn evaluate, OrderFn order, int depth, int ply, int alpha, int beta)
{
    if (search_should_stop()) return 0;

    // 1. Base Case (Horizon reached)
    // Instead of a static evaluation, we enter Quiescence Search to 
    // resolve any "noisy" tactical sequences (captures).
    if (depth <= 0) {
        return quiescence(board, alpha, beta, ply, evaluate, order_moves);
    }

    g_searchStats.nodes++;

    // 2. Move Generation & Ordering
    Moves moves = castro_GenerateMoves(board, MOVE_PSEUDO); 
    order(board, moves.list, moves.count, ply, NULL_MOVE);

    int legal_moves_count = 0;
    int max_eval = -INF;

    for (size_t i = 0; i < moves.count; i++) {
        if (!castro_MakeMove(board, moves.list[i])) continue;
        
        legal_moves_count++;
        
        int score = -negamax_rec(board, evaluate, order, depth - 1, ply + 1, -beta, -alpha);
        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        if (score > max_eval) max_eval = score;
        if (score > alpha)    alpha = score;
        
        // Alpha-Beta Cutoff
        if (alpha >= beta) break; 
    }

    // 3. Terminal Case (Checkmate / Stalemate)
    if (legal_moves_count == 0) {
        if (castro_IsInCheck(board)) 
            return -INF + ply; // Mate score adjusted by distance from root
        return 0; // Stalemate
    }

    return max_eval;
}
