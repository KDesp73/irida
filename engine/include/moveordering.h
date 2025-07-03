#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include "castro.h"
#include "tuning.h"

static const int PIECE_VALUES[] = {
    PAWN_VALUE,
    KNIGHT_VALUE,
    BISHOP_VALUE,
    ROOK_VALUE,
    QUEEN_VALUE,
    KING_VALUE
};

// See: https://www.chessprogramming.org/MVV-LVA
extern int MVV_LVA_SCORES[12][12];

static inline void InitMVVLVA() {
    for (int attacker = 0; attacker < 12; attacker++) {
        for (int victim = 0; victim < 12; victim++) {
            MVV_LVA_SCORES[attacker][victim] =
                PIECE_VALUES[victim % 6] * 10 - PIECE_VALUES[attacker % 6];
        }
    }
}

int ScoreMove(Board* board, Move move);
void SortMoves(Board* board, Moves* moves);

// TODO: 
// 1. Killer moves
// 2. History heuristics
// 3. Static Exchange Evaluation

#endif // MOVE_ORDERING_H
