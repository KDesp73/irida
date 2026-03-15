/*
 * Theory: Quiescence search.
 *
 * To avoid evaluating noisy positions (e.g. in the middle of a capture sequence),
 * we only consider capture moves. We evaluate the standing pat (current position);
 * if it fails high we return beta. Otherwise we generate captures, order them,
 * and search with negamax alpha-beta. The search is recursive until no captures
 * remain or we hit max ply. Mate scores are distance-adjusted so they stay
 * consistent across plies.
 */
#include "castro.h"
#include "eval.h"
#include "eval_cache.h"
#include "moveordering.h"
#include "search.h"

int quiescence(Board* board, int alpha, int beta, int ply, EvalFn eval, OrderFn order)
{
    g_searchStats.qnodes++;

    if (ply >= MAX_PLY)
        return eval_cached(board, eval);

    int stand_pat = eval_cached(board, eval);

    // Standing Pat: If current position is good enough to cause a cutoff, stop.
    if (stand_pat >= beta)
        return beta;

    // Delta Pruning: A queen is worth ~900. If we are down by 1000, 
    // even taking a queen won't save us.
    if (stand_pat < alpha - 900) { 
         if (!castro_IsInCheck(board)) {
            return alpha;
         }
    }

    if (stand_pat > alpha)
        alpha = stand_pat;

    Moves moves = castro_GenerateMoves(board, MOVE_CAPTURE);

    // If no captures are available, we return the standing pat
    if (moves.count == 0)
        return alpha;

    order(board, moves.list, moves.count, ply);

    for (size_t i = 0; i < moves.count; i++) {
        Move move = moves.list[i];

        // Legality check via MakeMove
        if (!castro_MakeMove(board, move))
            continue;

        int score = -quiescence(board, -beta, -alpha, ply + 1, eval, order);
        castro_UnmakeMove(board);

        if (score >= beta)
            return beta;

        if (score > alpha)
            alpha = score;
    }

    return alpha;
}
