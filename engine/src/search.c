#include "search.h"
#include "evaluation.h"
#include "move.h"
#include "movegen.h"
#include "moveordering.h"
#include "result.h"
#include <limits.h>
#include <stdio.h>

static int nodes = 0;

int Negamax(Board* board, int depth, int alpha, int beta)
{
    nodes++;

    if (depth == 0 || IsResult(board))
        return Evaluation(board).total;

    Moves moves = GenerateMoves(board, MOVE_LEGAL);
    SortMoves(board, &moves);

    int bestScore = INT_MIN;

    for (size_t i = 0; i < moves.count; i++) {
        Move move = moves.list[i];
        MakeMove(board, move);
        int score = -Negamax(board, depth - 1, -beta, -alpha);
        UnmakeMove(board);

        if (score > bestScore)
            bestScore = score;

        if (bestScore > alpha)
            alpha = bestScore;

        if (alpha >= beta)
            break;  // Beta cutoff
    }

    return bestScore;
}

Move FindBest(Board* board, int depth, int* outScore)
{
    nodes = 0;
    int alpha = INT_MIN + 1;
    int beta = INT_MAX;
    int bestScore = INT_MIN;
    Move bestMove = NULL_MOVE;

    Moves moves = GenerateMoves(board, MOVE_LEGAL);
    SortMoves(board, &moves);

    for (size_t i = 0; i < moves.count; i++) {
        Move move = moves.list[i];
        MakeMove(board, move);
        int score = -Negamax(board, depth - 1, -beta, -alpha);
        UnmakeMove(board);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;

            // Print UCI info line
            char mStr[16];
            MoveToString(bestMove, mStr);
            printf("info depth %d score cp %d nodes %d pv %s\n",
                   depth, bestScore, nodes, mStr);
            fflush(stdout);
        }

        if (bestScore > alpha)
            alpha = bestScore;
    }

    *outScore = bestScore;
    return bestMove;
}

Move FindBestIterative(Board* board, int maxDepth, int *outScore)
{
    Move bestMove = NULL_MOVE;
    int bestScore = INT_MIN;

    for (int depth = 1; depth <= maxDepth; depth++) {
        int score;
        nodes = 0;

        Move mv = FindBest(board, depth, &score);

        if (mv != NULL_MOVE) {
            bestMove = mv;
            bestScore = score;
        }
    }

    *outScore = bestScore;
    return bestMove;
}
