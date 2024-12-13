#include "move.h"

_Bool CanMoveQueen(const Board* board, Square from, Square to, uint8_t color) {
    return CanMoveRook(board, from, to, color) || CanMoveBishop(board, from, to, color);
}

