#include "castro.h"
#include "eval.h"
#include "moveordering.h"
#include "search.h"

#define MAX_DEPTH 64

int quiescence(Board* board,
               int alpha,
               int beta,
               int ply,
               EvalFn eval,
               OrderFn order)
{
    g_searchStats.qnodes++;

    if (ply >= MAX_PLY)
        return eval(board);

    int stand_pat = eval(board);

    // Mate score distance correction
    if (stand_pat > MATE_SCORE - MAX_PLY)
        stand_pat -= ply;
    if (stand_pat < -MATE_SCORE + MAX_PLY)
        stand_pat += ply;

    if (stand_pat >= beta)
        return beta;

    if (stand_pat > alpha)
        alpha = stand_pat;

    Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);

    size_t new_count = 0;

    for (size_t i = 0; i < moves.count; i++) {
        if (castro_IsCapture(board, moves.list[i])) {
            moves.list[new_count++] = moves.list[i];
        }
    }

    moves.count = new_count;

    if (moves.count == 0)
        return alpha;

    order(board, moves.list, moves.count, ply);

    for (size_t i = 0; i < moves.count; i++) {

        Move move = moves.list[i];

        if (!castro_MakeMove(board, move))
            continue;

        int score = -quiescence(board,
                                -beta,
                                -alpha,
                                ply + 1,
                                eval,
                                order);

        castro_UnmakeMove(board);

        if (score >= beta)
            return beta;

        if (score > alpha)
            alpha = score;
    }

    return alpha;
}
