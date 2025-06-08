#ifndef BITBOARD_H
#define BITBOARD_H

#include "square.h"
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

typedef uint64_t Bitboard;

typedef enum {
    NORTH = 8,
    SOUTH = -8,
    EAST = 1,
    WEST = -1,
    NORTH_EAST = 9,
    NORTH_WEST = 7,
    SOUTH_EAST = -7,
    SOUTH_WEST = -9
} Direction;

#define BB(square) ((square == 64) ? 0ULL : 1ULL << (square))

Square lsb(Bitboard b);
Square msb(Bitboard b);
Bitboard shift(Bitboard b, Direction D);
Square poplsb(Bitboard* b);
int popcount(Bitboard bb);
void on(Bitboard* bitboard, Square square);
void off(Bitboard* bitboard, Square square);

/*** Pseudo Legal Attack Maps ***/
Bitboard PawnAttacks(Square pawn, Bitboard enemySquares, uint8_t color);
Bitboard PawnPushes(Square pawn, Bitboard emptySquares, uint8_t color);
Bitboard PawnPromotions(Square pawns, Bitboard emptySquares, uint8_t color);
Bitboard PawnPromotionCaptures(Square pawns, Bitboard opponentPieces, uint8_t color);

Bitboard KnightAttacks(Square knights, Bitboard emptySquares, Bitboard enemySquares);
Bitboard KingAttacks(Square king, Bitboard emptySquares, Bitboard enemySquares);
Bitboard BishopAttacks(Square bishops, Bitboard emptySquares, Bitboard enemySquares);
Bitboard RookAttacks(Square rooks, Bitboard emptySquares, Bitboard enemySquares);
Bitboard QueenAttacks(Square queens, Bitboard emptySquares, Bitboard enemySquares);

/*** King Safety ***/
bool IsKingInCheck(Bitboard kingPosition, Bitboard enemyAttacks);
Bitboard _PinnedPieces(Bitboard kingPosition, Bitboard slidingAttacks, Bitboard occupancy);

/*** Printing ***/
void Uint32Print(uint32_t value);
void Uint64Print(uint64_t value);
void BitboardPrint(Bitboard bitboard);


#endif // BITBOARD_H

