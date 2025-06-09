#ifndef ENGINE_EVALUATION_H
#define ENGINE_EVALUATION_H

#include "board.h"
#include "tuning.h"

// https://www.chessprogramming.org/Evaluation

int EvaluateMaterial(const Board* board, const Tuning* tuning);
int EvaluatePieceSquareTables(const Board* board, const Tuning* tuning);
int EvaluateMobility(const Board* board, const Tuning* tuning, PieceColor color);
int EvaluateKingSafety(const Board* board, const Tuning* tuning);

// TODO: EvaluatePawnStructure(const Board* board, const Tuning* tuning);
// TODO: EvaluateThreats(const Board* board, const Tuning* tuning);


static inline int Evaluation(const Board* board)
{
    // FIXME: Temporary solution. Define tuning outside
    Tuning tuning = {0};
    LoadTuning(&tuning);

    int score = 0;

    score += EvaluateMaterial(board, &tuning);
    score += EvaluatePieceSquareTables(board, &tuning);
    score += EvaluateKingSafety(board, &tuning);
    score += EvaluateMobility(board, &tuning, board->turn);

    return score;
}

#endif // ENGINE_EVALUATION_H
