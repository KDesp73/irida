#include "castro.h"
#include "eval.h"
#include "search.h"
#include "uci.h"
#include <limits.h>

static int negamax_rec(Board* board, EvalFn evaluate, int depth, int ply, int alpha, int beta);

Move negamax_id_ab(Board* board, EvalFn eval, OrderFn order, SearchConfig* config) 
{
    Move best_move = NULL_MOVE;
    g_searchStats.nodes = 0;

    search_start_timer(config->timeLimitMs);

    // Iterative Deepening
    for (int currentDepth = 1; currentDepth <= config->maxDepth; currentDepth++) {
        
        if (search_time_up()) break;

        // 1. Generate moves once at the root
        Moves legal = castro_GenerateLegalMoves(board);

        // order(board, legal.list, legal.count, 0, best_move);

        int alpha = -INF;
        int beta = INF;
        int bestScore = -INF;
        Move currentBestMove = NULL_MOVE;

        for (size_t i = 0; i < legal.count; i++) {
            if (!castro_MakeMove(board, legal.list[i])) continue;

            // 3. Pass ply = 1 to the first recursive call
            int score = -negamax_rec(board, eval, currentDepth - 1, 1, -beta, -alpha);
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

static int negamax_rec(Board* board, EvalFn evaluate, int depth, int ply, int alpha, int beta)
{
    if (search_should_stop()) return 0;

    g_searchStats.nodes++;

    // 1. Terminal Check
    Moves moves = castro_GenerateMoves(board, MOVE_PSEUDO); 
    int legal_moves_count = 0;
    int max_eval = -INF;

    for (size_t i = 0; i < moves.count; i++) {
        if (!castro_MakeMove(board, moves.list[i])) continue;
        
        legal_moves_count++;
        // Note the addition of 'ply + 1'
        int score = -negamax_rec(board, evaluate, depth - 1, ply + 1, -beta, -alpha);
        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        if (score > max_eval) max_eval = score;
        if (score > alpha)    alpha = score;
        if (alpha >= beta) break; 
    }

    // 2. Terminal Case (Checkmate / Stalemate)
    if (legal_moves_count == 0) {
        if (castro_IsInCheck(board)) 
            return -INF + ply; // Shortest mate has highest score
        return 0; // Stalemate
    }

    // 3. Base Case (Leaf Node)
    if (depth <= 0) {
        return evaluate(board); 
    }

    return max_eval;
}
