#ifndef ENGINE_MASKS_H
#define ENGINE_MASKS_H

#include <stdint.h>

typedef uint64_t Bitboard;

static Bitboard DIAGONAL_MASKS[64];
static Bitboard ANTI_DIAGONAL_MASKS[64];

typedef enum {
    DIAGONAL,
    VERTICAL,
    HORIZONTAL
} RayType;

Bitboard GeneralOccupancy(Bitboard whitePieces, Bitboard blackPieces);
Bitboard BlockerMasks(Bitboard slidingPiece, Bitboard occupancy);

// Helper function to compute diagonal mask for a given square
Bitboard ComputeDiagonalMask(int square);
// Helper function to compute anti-diagonal mask for a given square
Bitboard ComputeAntiDiagonalMask(int square);
// Precompute diagonal and anti-diagonal masks for all squares
void InitializeMasks();

// Accessors for diagonal and anti-diagonal masks
Bitboard DiagonalMask(int square);
Bitboard AntiDiagonalMask(int square);


#endif // ENGINE_MASKS_H
