#ifndef ENGINE_PIECE_H
#define ENGINE_PIECE_H

#include "board.h"
#include "square.h"
#include <stdint.h>
typedef struct {
    char type;
    PieceColor color;
} Piece;

#define PIECE_PRINT(piece)\
    printf("%s = {.type=%c, .color=%d}\n", #piece, piece.type, piece.color)


#define IS_PAWN(piece) \
    (tolower(piece.type) == 'p')
#define IS_KNIGHT(piece) \
    (tolower(piece.type) == 'n')
#define IS_BISHOP(piece) \
    (tolower(piece.type) == 'b')
#define IS_ROOK(piece) \
    (tolower(piece.type) == 'r')
#define IS_QUEEN(piece) \
    (tolower(piece.type) == 'q')
#define IS_KING(piece) \
    (tolower(piece.type) == 'k')
#define IS_WHITE(piece) \
    (piece.color == COLOR_WHITE)
#define IS_BLACK(piece) \
    (piece.color == COLOR_BLACK)


int GetPieceColor(char piece);
Piece PieceAt(const Board* board, Square square);
bool PieceCmp(Piece p1, Piece p2);

#endif // ENGINE_PIECE_H
