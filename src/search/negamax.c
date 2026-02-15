#include "castro.h"
#include "eval.h"

#define INF 1000000

int negamax(Board* board, int depth, int alpha, int beta, EvalFn eval)
{
    if (depth == 0) {
        return eval(board);
    }

    Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);

    if (moves.count == 0) {
        // Checkmate or stalemate
        if (castro_IsInCheck(board))
            return -INF + 1;  // mate score
        return 0;            // stalemate
    }

    int bestScore = -INF;

    for (int i = 0; i < moves.count; i++) {

        Move move = moves.list[i];

        if (!castro_MakeMove(board, move))
            continue;  // illegal (e.g., leaves king in check)

        int score = -negamax(board, depth - 1, -beta, -alpha, eval);

        castro_UnmakeMove(board);

        if (score > bestScore)
            bestScore = score;

        if (score > alpha)
            alpha = score;

        if (alpha >= beta)
            break;  // beta cutoff
    }

    return bestScore;
}

Move alpha_beta_search(Board* board, EvalFn eval, int depth)
{
    Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);

    Move bestMove = {0};
    int bestScore = -INF;

    for (int i = 0; i < moves.count; i++) {

        Move move = moves.list[i];

        if (!castro_MakeMove(board, move))
            continue;

        int score = -negamax(board, depth - 1, -INF, INF, eval);

        castro_UnmakeMove(board);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }

    return bestMove;
}

