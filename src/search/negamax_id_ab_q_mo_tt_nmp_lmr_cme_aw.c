#include "castro.h"
#include "castro_additions.h"
#include "eval.h"
#include "search.h"
#include "uci.h"
#include "moveordering.h"
#include "tt.h"
#include <stdio.h>

static int aspiration_prev_ok(int prevScore)
{
    return prevScore > -MATE_SCORE / 2 && prevScore < MATE_SCORE / 2;
}

static int negamax_rec(Board* board,
                       EvalFn eval,
                       OrderFn order,
                       int depth,
                       int ply,
                       int alpha,
                       int beta,
                       SearchConfig* config);

Move negamax_id_ab_q_mo_tt_nmp_lmr_cme_aw(Board* board, EvalFn eval, OrderFn order, SearchConfig* config)
{
    Move best_move = NULL_MOVE;
    g_searchStats.nodes = 0;
    g_searchStats.qnodes = 0;

    search_start_timer(config->timeLimitMs);

    int prevScore = 0;

    for (int currentDepth = 1; currentDepth <= config->maxDepth; currentDepth++) {

        if (search_time_up()) break;

        Moves legal = castro_GenerateMoves(board, MOVE_LEGAL);

        order(board, legal.list, legal.count, 0, NULL_MOVE);

        int windowAlpha = -INF;
        int windowBeta = INF;
        if (config->useAspiration && currentDepth > 1 && aspiration_prev_ok(prevScore)) {
            int margin = 25 + currentDepth * 8;
            if (margin > 500) margin = 500;
            windowAlpha = prevScore - margin;
            windowBeta = prevScore + margin;
        }

        int bestScore = -INF;
        Move currentBestMove = NULL_MOVE;

        for (;;) {
            int alpha = windowAlpha;
            int beta = windowBeta;
            bestScore = -INF;

            for (size_t i = 0; i < legal.count; i++) {
                if (!castro_MakeMove(board, legal.list[i])) continue;

                int score = -negamax_rec(board, eval, order, currentDepth - 1, 1, -beta, -alpha, config);

                castro_UnmakeMove(board);

                if (search_should_stop()) break;

                if (score > bestScore) {
                    bestScore = score;
                    currentBestMove = legal.list[i];
                }
                if (score > alpha) alpha = score;
            }

            if (search_should_stop()) break;

            if (config->useAspiration && currentDepth > 1 && aspiration_prev_ok(prevScore)) {
                if (bestScore <= windowAlpha && windowAlpha > -INF) {
                    windowAlpha = -INF;
                    continue;
                }
                if (bestScore >= windowBeta && windowBeta < INF) {
                    windowBeta = INF;
                    continue;
                }
            }
            break;
        }

        if (!search_should_stop()) {
            best_move = currentBestMove;
            prevScore = bestScore;
            char moveBuf[10];
            castro_MoveToString(best_move, moveBuf);

            uci_report_search(currentDepth, bestScore, g_searchStats.nodes + g_searchStats.qnodes, search_elapsed_ms(), moveBuf);

            if (bestScore > (INF - MAX_PLY)) break;
        } else {
            break;
        }
    }

    return best_move;
}

static int negamax_rec(Board* board,
                       EvalFn eval,
                       OrderFn order,
                       int depth,
                       int ply,
                       int alpha,
                       int beta,
                       SearchConfig* config)
{
    int tt_score = 0;
    Move tt_move = NULL_MOVE;
    if (tt_probe(board->hash, depth, alpha, beta, ply, &tt_score, &tt_move)) {
        return tt_score;
    }

    if ((g_searchStats.nodes & 2047) == 0) {
        if (search_time_up()) {
            uci_state.stopRequested = true;
        }
    }
    if (search_should_stop()) return 0;

    g_searchStats.nodes++;

    Moves legal = castro_GenerateMoves(board, MOVE_LEGAL);

    if (depth <= 0) {
        return quiescence(board, alpha, beta, ply, eval, order);
    }

    if (depth >= 3 && !castro_IsInCheck(board) && castro_HasNonPawnMaterial(board)) {
        castro_MakeNullMove(board);
        int nullScore = -negamax_rec(board, eval, order, depth - 1 - 3, ply + 1, -beta, -beta + 1, config);
        castro_UnmakeNullMove(board);

        if (nullScore >= beta) {
            if (nullScore > MATE_SCORE) nullScore = beta;
            return nullScore;
        }
    }

    int original_alpha = alpha;
    Move best_move_found = NULL_MOVE;
    int legal_moves_count = 0;
    const bool parent_in_check = castro_IsInCheck(board);

    order(board, legal.list, legal.count, ply, tt_move);

    int max_eval = -INF;
    for (size_t i = 0; i < legal.count; i++) {
        Move move = legal.list[i];
        bool is_capture = castro_IsCapture(board, move);

        if (!castro_MakeMove(board, move)) continue;

        legal_moves_count++;
        const size_t move_index = (size_t)legal_moves_count - 1;

        const bool gives_check = castro_IsInCheck(board);
        int next_depth = depth - 1 + (gives_check ? 1 : 0);

        int score;
        const bool use_lmr = config->useLMR && depth >= 3 && !parent_in_check && !is_capture && !gives_check
            && move_index >= 4;

        if (use_lmr) {
            int R = 1;
            if (depth >= 6 && move_index >= 6) R++;
            if (depth >= 10 && move_index >= 10) R++;

            int reduced_depth = next_depth - R;
            if (reduced_depth < 0) reduced_depth = 0;

            score = -negamax_rec(board, eval, order, reduced_depth, ply + 1, -beta, -alpha, config);

            if (reduced_depth < next_depth && (score > alpha || score >= beta)) {
                score = -negamax_rec(board, eval, order, next_depth, ply + 1, -beta, -alpha, config);
            }
        } else {
            score = -negamax_rec(board, eval, order, next_depth, ply + 1, -beta, -alpha, config);
        }

        castro_UnmakeMove(board);

        if (search_should_stop()) return 0;

        if (score > max_eval) {
            max_eval = score;
            best_move_found = move;
        }

        if (score > alpha) alpha = score;

        if (alpha >= beta) break;
    }

    if (legal_moves_count == 0) {
        if (castro_IsInCheck(board)) {
            return -INF + ply;
        }
        return 0;
    }

    TTNodeType type = TT_EXACT;
    if (max_eval <= original_alpha) type = TT_UPPERBOUND;
    else if (max_eval >= beta) type = TT_LOWERBOUND;

    tt_store(board->hash, depth, max_eval, type, best_move_found, ply);

    return max_eval;
}
