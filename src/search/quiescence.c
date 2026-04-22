/*
 * Quiescence search.
 *
 * To avoid evaluating noisy positions (e.g. in the middle of a capture sequence),
 * we only consider capture moves. We evaluate the standing pat (current position);
 * if it fails high we return beta. Otherwise we generate captures, order them,
 * and search with negamax alpha-beta. The search is recursive until no captures
 * remain or we hit max ply. Mate scores are distance-adjusted so they stay
 * consistent across plies.
 *
 * See https://www.chessprogramming.org/Quiescence_Search
 */
#include "castro.h"
#include "eval.h"
#include "moveordering.h"
#include "search.h"
#include "uci.h"
#include "nnue.h"

int irida_Quiescence(Board* board, int alpha, int beta, int ply, EvalFn eval, OrderFn order)
{
    g_searchStats.qnodes++;
    if (ply > g_searchStats.selDepth)
        g_searchStats.selDepth = ply;

    /* Quiescence can explode on capture sequences; negamax only polls time/stop
     * every N nodes at its own ply. Poll here so movetime / quit are honored. */
    if ((g_searchStats.qnodes & 1023u) == 0u && irida_SearchTimeUp())
        uci_state.stopRequested = true;
    if (irida_SearchShouldStop())
        return alpha;

    // Pseudocode doesn't define MAX_PLY, but we keep it for safety
    if (ply >= MAX_PLY)
        return eval(board);

    // int static_eval = Evaluate();
    int static_eval = eval(board);

    // Stand Pat
    int best_value = static_eval;

    // if( best_value >= beta ) return best_value;
    if (best_value >= beta)
        return best_value;

    // if( best_value > alpha ) alpha = best_value;
    if (best_value > alpha)
        alpha = best_value;

    Moves moves = castro_GenerateMoves(board, MOVE_CAPTURE);
    order(board, moves.list, moves.count, ply, NULL_MOVE);

    const bool use_acc = g_searchConfig.useNNUEAccumulator && irida_NNUEAvailable() && eval == irida_EvalNNUE;

    // until( every_capture_has_been_examined )
    for (size_t i = 0; i < moves.count; i++) {
        Move move = moves.list[i];

        if (irida_SearchShouldStop())
            return alpha;

        if (use_acc) irida_NNUEAccBeforeChild(board, move);
        if (!castro_MakeMove(board, move))
            continue;
        if (use_acc) irida_NNUEAccCommitChild();

        // score = -Quiesce( -beta, -alpha );
        int score = -irida_Quiescence(board, -beta, -alpha, ply + 1, eval, order);
        castro_UnmakeMove(board);
        if (use_acc) irida_NNUEAccPop();

        // if( score >= beta ) return score;
        if (score >= beta)
            return score;

        // if( score > best_value ) best_value = score;
        if (score > best_value)
            best_value = score;

        // if( score > alpha ) alpha = score;
        if (score > alpha)
            alpha = score;
    }

    return best_value;
}
