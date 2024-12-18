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

#define FILE_A  0x0101010101010101ULL  // File A (a1, a2, ..., a8)
#define FILE_B  0x0202020202020202ULL  // File B (b1, b2, ..., b8)
#define FILE_C  0x0404040404040404ULL  // File C (c1, c2, ..., c8)
#define FILE_D  0x0808080808080808ULL  // File D (d1, d2, ..., d8)
#define FILE_E  0x1010101010101010ULL  // File E (e1, e2, ..., e8)
#define FILE_F  0x2020202020202020ULL  // File F (f1, f2, ..., f8)
#define FILE_G  0x4040404040404040ULL  // File G (g1, g2, ..., g8)
#define FILE_H  0x8080808080808080ULL  // File H (h1, h2, ..., h8)

#define RANK_1  0x00000000000000FFULL  // Rank 1 (a1-h1)
#define RANK_2  0x000000000000FF00ULL  // Rank 2 (a2-h2)
#define RANK_3  0x0000000000FF0000ULL  // Rank 3 (a3-h3)
#define RANK_4  0x00000000FF000000ULL  // Rank 4 (a4-h4)
#define RANK_5  0x000000FF00000000ULL  // Rank 5 (a5-h5)
#define RANK_6  0x0000FF0000000000ULL  // Rank 6 (a6-h6)
#define RANK_7  0x00FF000000000000ULL  // Rank 7 (a7-h7)
#define RANK_8  0xFF00000000000000ULL  // Rank 8 (a8-h8)

Square lsb(Bitboard b);
Square msb(Bitboard b);
Bitboard shift(Bitboard b, Direction D);
Square poplsb(Bitboard* b);
int popcount(Bitboard bb);
void on(Bitboard* bitboard, Square square);
void off(Bitboard* bitboard, Square square);

/*** Pseudo Valid Attack Maps ***/
Bitboard PawnAttacks(Bitboard pawns, Bitboard enemySquares, uint8_t color);
Bitboard WhitePawnAttacks(Bitboard pawns, Bitboard enemySquares);
Bitboard BlackPawnAttacks(Bitboard pawns, Bitboard enemySquares);

Bitboard PawnPushes(Bitboard pawns, Bitboard emptySquares, uint8_t color);
Bitboard WhitePawnPushes(Bitboard pawns, Bitboard emptySquares);
Bitboard BlackPawnPushes(Bitboard pawns, Bitboard emptySquares);

Bitboard PawnPromotions(Bitboard pawns, Bitboard emptySquares, uint8_t color);
Bitboard WhitePawnPromotions(Bitboard pawns, Bitboard emptySquares);
Bitboard BlackPawnPromotions(Bitboard pawns, Bitboard emptySquares);

Bitboard PawnPromotionCaptures(Bitboard pawns, Bitboard opponentPieces, uint8_t color);
Bitboard WhitePawnPromotionCaptures(Bitboard pawns, Bitboard opponentPieces);
Bitboard BlackPawnPromotionCaptures(Bitboard pawns, Bitboard opponentPieces);

Bitboard KnightAttacks(Bitboard knights, Bitboard emptySquares, Bitboard enemySquares);
Bitboard KingAttacks(Bitboard king, Bitboard emptySquares, Bitboard enemySquares);
Bitboard BishopAttacks(Bitboard bishops, Bitboard emptySquares, Bitboard enemySquares);
Bitboard RookAttacks(Bitboard rooks, Bitboard emptySquares, Bitboard enemySquares);
Bitboard QueenAttacks(Bitboard queens, Bitboard emptySquares, Bitboard enemySquares);

/*** King Safety ***/
bool IsKingInCheck(Bitboard kingPosition, Bitboard enemyAttacks);
Bitboard PinnedPieces(Bitboard kingPosition, Bitboard slidingAttacks, Bitboard occupancy);

/*** Special Moves ***/
Bitboard GenerateCastlingMoves(Bitboard kingPosition, bool canCastleKingside, bool canCastleQueenside, Bitboard enemyAttacks);
Bitboard GenerateEnPassantMoves(Bitboard pawns, Bitboard enPassantSquare, Bitboard enemyPieces);

/*** Legal Move Validation ***/
bool IsMoveLegal(Bitboard kingPosition, Bitboard move, Bitboard enemyAttacks);


/*** Printing ***/
void Uint32Print(uint32_t value);
void Uint64Print(uint64_t value);
void BitboardPrint(Bitboard bitboard);


#endif // BITBOARD_H

