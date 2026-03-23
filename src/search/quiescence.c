/*
 * Quiescence search extends the main search past depth 0 so evaluation is not
 * applied in the middle of a tactical sequence (horizon effect).
 *
 * Not in check: standing pat (eval) establishes a floor; only captures are
 * expanded (with delta pruning: if we are far below alpha, no capture fixes it).
 * In check: there is no legal stand-pat, so all legal moves are searched—
 * otherwise mate threats after “quiet” positions would be missed.
 * Terminal: no moves in check is mate (-INF + ply); stalemate returns 0.
 */
#include "castro.h"
#include "eval.h"
#include "moveordering.h"
#include "search.h"

/* Negamax quiescence with alpha-beta; same eval/ordering hooks as main search. */
int quiescence(Board* board, int alpha, int beta, int ply, EvalFn eval, OrderFn order)
{
    g_searchStats.qnodes++;

    if (ply >= MAX_PLY)
        return eval(board);

    const bool in_check = castro_IsInCheck(board);

    if (!in_check) {
        int stand_pat = eval(board);

        if (stand_pat >= beta)
            return beta;

        // Delta pruning: a queen is worth ~900; if we are down by ~1000,
        // even taking a queen will not reach alpha.
        if (stand_pat < alpha - 900)
            return alpha;

        if (stand_pat > alpha)
            alpha = stand_pat;
    }

    Moves moves;
    if (in_check) {
        moves = castro_GenerateMoves(board, MOVE_LEGAL);
        if (moves.count == 0)
            return -INF + ply;
    } else {
        moves = castro_GenerateMoves(board, MOVE_CAPTURE);
        if (moves.count == 0)
            return alpha;
    }

    order(board, moves.list, moves.count, ply, NULL_MOVE);

    for (size_t i = 0; i < moves.count; i++) {
        Move move = moves.list[i];

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
