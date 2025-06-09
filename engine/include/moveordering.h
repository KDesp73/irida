#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include "board.h"
#include "move.h"
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
int MVV_LVA_SCORES[12][12];

static inline void InitMVVLVA() {
    for (int attacker = 0; attacker < 12; attacker++) {
        for (int victim = 0; victim < 12; victim++) {
            MVV_LVA_SCORES[attacker][victim] =
                PIECE_VALUES[victim % 6] * 10 - PIECE_VALUES[attacker % 6];
        }
    }
}

// TODO: Killer moves
// TODO: History heuristics
int ScoreMove(Board* board, Move move);

void SortMoves(Board* board, Moves* moves);

#endif // MOVE_ORDERING_H
