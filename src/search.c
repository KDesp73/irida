#include "search.h"
#include "evaluation.h"
#include "move.h"
#include "movegen.h"
#include "result.h"
#include <math.h>

static int max(int a, int b) 
{
    return (a < b) ? b : a;
}
static int min(int a, int b) 
{
    return (a < b) ? a : b;
}

int Minimax(Board* board, int depth, bool isMaximizing)
{
    if(depth == 0 || IsResult(board))
        return Evaluation(board);

    int bestScore = isMaximizing ? -INFINITY : INFINITY;
    Moves moves = GenerateLegalMoves(board);

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

Move FindBest(Board* board, int depth)
{
    int bestScore = -INFINITY;
    Move bestMove;
    Moves moves = GenerateLegalMoves(board);

    for (size_t i = 0; i < moves.count; i++) {
        Move move  = moves.list[i];
        MakeMove(board, move);
        int score = Minimax(board, depth - 1, false);
        UnmakeMove(board);

        if (score > bestScore) {
            bestScore = score;
            bestMove = move;
        }
    }
    return bestMove;
}

