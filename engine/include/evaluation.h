#ifndef ENGINE_EVALUATION_H
#define ENGINE_EVALUATION_H

#include "board.h"
#include "tuning.h"

// https://www.chessprogramming.org/Evaluation

int EvaluateMaterial(const Board* board, const Tuning* tuning);
int EvaluatePieceSquareTables(const Board* board, const Tuning* tuning);
int EvaluateMobility(const Board* board, const Tuning* tuning, PieceColor color);
int EvaluateKingSafety(const Board* board, const Tuning* tuning, PieceColor color);

// TODO: EvaluatePawnStructure(const Board* board, const Tuning* tuning, PieceColor color);
// TODO: EvaluateThreats(const Board* board, const Tuning* tuning, PieceColor);


static inline int Evaluation(const Board* board)
{
    Tuning tuning = {0};
    LoadTuning(&tuning);

    int score = 0;

    score += EvaluateMaterial(board, &tuning);
    score += EvaluatePieceSquareTables(board, &tuning);
    
    score += EvaluateKingSafety(board, &tuning, COLOR_WHITE);
    score -= EvaluateKingSafety(board, &tuning, COLOR_BLACK);

    score += EvaluateMobility(board, &tuning, COLOR_WHITE);
    score -= EvaluateMobility(board, &tuning, COLOR_BLACK);

    // Tempo bonus
    score += (board->turn == COLOR_WHITE ? +10 : -10);

    // Bishop pair bonus
    if (popcount(board->bitboards[COLOR_WHITE * 6 + INDEX_BISHOP]) >= 2)
        score += 30;
    if (popcount(board->bitboards[COLOR_BLACK * 6 + INDEX_BISHOP]) >= 2)
        score -= 30;

    return score;
}

#endif // ENGINE_EVALUATION_H
