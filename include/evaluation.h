#ifndef ENGINE_EVALUATION_H
#define ENGINE_EVALUATION_H

#include "board.h"

static const int PAWN_VALUE = 100;
static const int KNIGHT_VALUE = 320;
static const int BISHOP_VALUE = 330;
static const int ROOK_VALUE = 500;
static const int QUEEN_VALUE = 900;
static const int KING_VALUE = 20000;


// default evaluation function
#define Evaluation EvaluateMaterial

int EvaluateMaterial(const Board* board);
// TODO: better evaluation methods

#endif // ENGINE_EVALUATION_H

