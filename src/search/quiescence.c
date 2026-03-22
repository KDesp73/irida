/*
 * Theory: Quiescence search.
 *
 * Off check, we evaluate a standing pat and only expand captures (plus delta
 * pruning). In check there is no legal stand-pat: we expand all legal moves so
 * non-capture evasions are not missed. Checkmate leaf matches main search:
 * -INF + ply.
 */
#include "castro.h"
#include "eval.h"
#include "moveordering.h"
#include "search.h"

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
