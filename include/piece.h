#ifndef ENGINE_PIECE_H
#define ENGINE_PIECE_H

#include "board.h"
#include "square.h"
#include <stdint.h>
typedef struct {
    char type;
    Color color;
} Piece;

#define PIECE_PRINT(piece)\
    printf("%s = {.type=%c, .color=%d}\n", #piece, piece.type, piece.color)

Piece PieceAt(const Board* board, Square square);

#endif // ENGINE_PIECE_H
