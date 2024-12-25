#ifndef ENGINE_MASKS_H
#define ENGINE_MASKS_H

#include "board.h"
#include "square.h"
#include <stdint.h>

typedef uint64_t Bitboard;

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

static Bitboard DIAGONAL_MASKS[64];
static Bitboard ANTI_DIAGONAL_MASKS[64];
static Bitboard HORIZONTAL_MASKS[64];
static Bitboard VERTICAL_MASKS[64];

static Bitboard PAWN_PUSH_MASKS[2][64];
static Bitboard PAWN_DOUBLE_PUSH_MASKS[2][64];
static Bitboard PAWN_ATTACK_MASKS[2][64];
static Bitboard PAWN_PROMOTION_MASKS[2][64];
static Bitboard PAWN_PROMOTION_ATTACK_MASKS[2][64];

static Bitboard KNIGHT_MOVE_MASKS[64];
static Bitboard BISHOP_MOVE_MASKS[64];
static Bitboard ROOK_MOVE_MASKS[64];
static Bitboard QUEEN_MOVE_MASKS[64];
static Bitboard KING_MOVE_MASKS[64];

typedef enum {
    DIAGONAL,
    VERTICAL,
    HORIZONTAL
} RayType;

void InitMasks();

Bitboard GeneralOccupancy(Bitboard whitePieces, Bitboard blackPieces);
Bitboard BlockerMasks(Bitboard slidingPiece, Bitboard occupancy);

/*** Ray Masks ***/
Bitboard ComputeDiagonalMask(Square square);
Bitboard ComputeAntiDiagonalMask(Square square);
Bitboard ComputeHorizontalMask(Square);
Bitboard ComputeVerticalMask(Square);

Bitboard DiagonalMask(Square square);
Bitboard AntiDiagonalMask(Square square);
Bitboard HorizontalMask(Square square);
Bitboard VerticalMask(Square square);

/*** Piece Masks ***/
Bitboard ComputePawnPushMask(Square square, PieceColor color);
Bitboard ComputePawnDoublePushMask(Square square, PieceColor color);
Bitboard ComputePawnPromotionMask(Square square, PieceColor color);
Bitboard ComputePawnPromotionAttackMask(Square square, PieceColor color);
Bitboard ComputePawnAttackMask(Square square, PieceColor color);
Bitboard ComputeKnightMoveMask(Square square);
Bitboard ComputeBishopMoveMask(Square square);
Bitboard ComputeRookMoveMask(Square square);
Bitboard ComputeQueenMoveMask(Square square);
Bitboard ComputeKingMoveMask(Square square);

Bitboard PawnPushMask(Square square, PieceColor color);
Bitboard PawnDoublePushMask(Square square, PieceColor color);
Bitboard PawnPromotionMask(Square square, PieceColor color);
Bitboard PawnPromotionAttackMask(Square square, PieceColor color);
Bitboard PawnAttackMask(Square square, PieceColor color);
Bitboard KnightMoveMask(Square square);
Bitboard BishopMoveMask(Square square);
Bitboard RookMoveMask(Square square);
Bitboard QueenMoveMask(Square square);
Bitboard KingMoveMask(Square square);


#endif // ENGINE_MASKS_H
