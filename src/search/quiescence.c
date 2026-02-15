#include "castro.h"
#include "eval.h"
#include "moveordering.h"

#define MAX_DEPTH 64

int quiescence(Board* board, int alpha, int beta, EvalFn eval)
{
    int stand_pat = eval(board);
    if (stand_pat >= beta) return beta;
    if (stand_pat > alpha) alpha = stand_pat;

    Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);

    // Keep only captures
    size_t new_count = 0;
    for (size_t i = 0; i < moves.count; i++) {
        if (castro_IsCapture(board, moves.list[i])) {
            moves.list[new_count++] = moves.list[i];
        }
    }
    moves.count = new_count;

    order_moves(board, moves.list, moves.count, 0);

    for (int i = 0; i < moves.count; i++) {
        Move move = moves.list[i];
        if (!castro_MakeMove(board, move)) continue;

        int score = -quiescence(board, -beta, -alpha, eval);
        castro_UnmakeMove(board);

        if (score >= beta)
            return beta;
        if (score > alpha)
            alpha = score;
    }

    return alpha;
}

