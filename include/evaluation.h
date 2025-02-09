#ifndef ENGINE_EVALUATION_H
#define ENGINE_EVALUATION_H

#include "board.h"

static const int PAWN_VALUE = 100;
static const int KNIGHT_VALUE = 320;
static const int BISHOP_VALUE = 330;
static const int ROOK_VALUE = 500;
static const int QUEEN_VALUE = 900;
static const int KING_VALUE = 20000;

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

