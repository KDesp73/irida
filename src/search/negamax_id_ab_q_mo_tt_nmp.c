#include "castro.h"
#include "castro_additions.h"
#include "eval.h"
#include "search.h"
#include "uci.h"
#include "moveordering.h"
#include "tt.h"
#include <limits.h>
#include <stdio.h>

static int negamax_rec(Board* board, EvalFn eval, OrderFn order, int depth, int ply, int alpha, int beta);

Move negamax_id_ab_q_mo_tt_nmp(Board* board, EvalFn eval, OrderFn order, SearchConfig* config) 
{
    Move best_move = NULL_MOVE;
    g_searchStats.nodes = 0;
    g_searchStats.qnodes = 0;

    search_start_timer(config->timeLimitMs);

    for (int currentDepth = 1; currentDepth <= config->maxDepth; currentDepth++) {
        
        if (search_time_up()) break;

        Moves legal = castro_GenerateMoves(board, MOVE_LEGAL);
        
        // Use the order function with ply 0 at the root
        order(board, legal.list, legal.count, 0, NULL_MOVE);

        int alpha = -INF;
        int beta = INF;
        int bestScore = -INF;
        Move currentBestMove = NULL_MOVE;

        for (size_t i = 0; i < legal.count; i++) {
            if(!castro_MakeMove(board, legal.list[i])) continue;
            
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

static int negamax_rec(Board* board, EvalFn eval, OrderFn order, int depth, int ply, int alpha, int beta)
{
    // 0. TT Probe
    int tt_score = 0;
    Move tt_move = NULL_MOVE;
    // Probe the table. If a valid entry exists with sufficient depth, return it immediately.
    // NOTE: This also retrieves the tt_move to help with move ordering.
    if (tt_probe(board->hash, depth, alpha, beta, ply, &tt_score, &tt_move)) {
        return tt_score;
    }

    // Periodically check timer every 2048 nodes
    if ((g_searchStats.nodes & 2047) == 0) {
        if (search_time_up()) {
            uci_state.stopRequested = true;
        }
    }
    if (search_should_stop()) return 0;

    g_searchStats.nodes++;

    // 1. Terminal Check
    Moves legal = castro_GenerateMoves(board, MOVE_LEGAL);

    // 2. Base Case: Transition to Quiescence Search
    if (depth <= 0) {
        return quiescence(board, alpha, beta, ply, eval, order);
    }

    // NMP
    if (
        depth >= 3
        && !castro_IsInCheck(board)
        && castro_HasNonPawnMaterial(board)
    ) {
        castro_MakeNullMove(board);
        // Using a reduced depth (R=3) and a null window (-beta, -beta + 1)
        int nullScore = -negamax_rec(board, eval, order, depth - 1 - 3, ply + 1, -beta, -beta + 1);
        castro_UnmakeNullMove(board);

        if (nullScore >= beta) {
            // Clamp mate scores: we can't prove a mate via NMP
            if (nullScore > MATE_SCORE) nullScore = beta;
            return nullScore; 
        }
    }

    // --- Before Search ---
    int original_alpha = alpha;
    Move best_move_found = NULL_MOVE;
    int legal_moves_count = 0;

    // Use tt_move for ordering if it exists (it's the most likely move to cause a cutoff)
    order(board, legal.list, legal.count, ply, tt_move); 

    int max_eval = -INF;
    for (size_t i = 0; i < legal.count; i++) {
        if(!castro_MakeMove(board, legal.list[i])) continue;

        legal_moves_count++;
        int score = -negamax_rec(board, eval, order, depth - 1, ply + 1, -beta, -alpha);
        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        if (score > max_eval) {
            max_eval = score;
            best_move_found = legal.list[i];
        }

        if (score > alpha) alpha = score;

        // Alpha-Beta Cutoff
        if (alpha >= beta) break; 
    }

    if (legal_moves_count == 0) {
        if (castro_IsInCheck(board)) {
            return -INF + ply; 
        }
        return 0; // Stalemate
    }

    // Determine the type of node we just searched:
    // 1. EXACT: Score was within the alpha-beta window.
    // 2. LOWERBOUND (Fail-High): Score was >= beta; opponent will avoid this branch.
    // 3. UPPERBOUND (Fail-Low): Score was <= original_alpha; this branch is bad for us.
    TTNodeType type = TT_EXACT;
    if (max_eval <= original_alpha) type = TT_UPPERBOUND;
    else if (max_eval >= beta)      type = TT_LOWERBOUND;

    tt_store(board->hash, depth, max_eval, type, best_move_found, ply);

    return max_eval;
}
