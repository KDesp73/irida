#ifndef ENGINE_PIECE_H
#define ENGINE_PIECE_H

#include "board.h"
#include "square.h"
typedef struct {
    char type;
    uint8_t color;
} Piece;
#define PIECE_PRINT(piece)\
    printf("%s = {.type=%c, .color=%d}\n", #piece, piece.type, piece.color)

Piece PieceAt(const Board* board, Square square);
_Bool PieceIsPinned(Board* board, Square piece);
_Bool PieceCanAttack(Board* board, Square attacker, Square target, _Bool strict);

#endif // ENGINE_PIECE_H
