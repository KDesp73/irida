#include "search.h"
#include "evaluation.h"
#include "move.h"
#include "movegen.h"
#include "moveordering.h"
#include "result.h"
#include <limits.h>
#include <stdio.h>

static int max(int a, int b) 
{
    return (a < b) ? b : a;
}
static int min(int a, int b) 
{
    return (a < b) ? a : b;
}

static int nodes = 0;
int Minimax(Board* board, int depth, bool isMaximizing)
{
    nodes++;
    if(depth == 0 || IsResult(board))
        return Evaluation(board).total;

    int bestScore = isMaximizing ? INT_MIN : INT_MAX;
    Moves moves = GenerateMoves(board, MOVE_LEGAL);
    SortMoves(board, &moves);

    for(size_t i = 0; i < moves.count; i++){
        Move move = moves.list[i];
        
        MakeMove(board, move);
        int score = Minimax(board, depth - 1, !isMaximizing);
        UnmakeMove(board);

        if(isMaximizing)
            bestScore = max(bestScore, score);
        else
            bestScore = min(bestScore, score);
    }
    return bestScore;
}

Move FindBest(Board* board, int depth, int* score)
{
    nodes = 0;
    int bestScore = INT_MIN;
    Move bestMove = NULL_MOVE;
    Moves moves = GenerateMoves(board, MOVE_LEGAL);
    SortMoves(board, &moves);

    for (size_t i = 0; i < moves.count; i++) {
        Move move  = moves.list[i];
        MakeMove(board, move);
        int moveScore = Minimax(board, depth - 1, false);
        UnmakeMove(board);

        if (moveScore > bestScore) {
            bestScore = moveScore;
            bestMove = move;

            // Print UCI "info" line
            char mStr[16];
            MoveToString(bestMove, mStr);
            printf("info depth %d score cp %d nodes %d pv %s\n", 
                   depth, bestScore, nodes, mStr);
            fflush(stdout);
        }
    }

    *score = bestScore;
    return bestMove;
}

