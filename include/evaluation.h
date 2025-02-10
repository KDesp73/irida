#ifndef ENGINE_EVALUATION_H
#define ENGINE_EVALUATION_H

#include "board.h"

// https://www.chessprogramming.org/Evaluation

int EvaluateMaterial(const Board* board);
int EvaluatePieceSquareTables(const Board* board);
int EvaluateMobility(const Board* board, PieceColor color);

int EvaluateKingSafety(const Board* board); // TODO: Implement
// TODO: EvaluatePawnStructure
// TODO: EvaluateThreats
// TODO: Fine-Tuning


static inline int Evaluation(const Board* board)
{
    int score = 0;

    score += EvaluateMaterial(board);
    score += EvaluatePieceSquareTables(board);
    score += EvaluateKingSafety(board);
    score += EvaluateMobility(board, board->turn);

    return score;
}

#endif // ENGINE_EVALUATION_H

