#include "masks.h"
#include "bitboard.h"

Bitboard GeneralOccupancy(Bitboard whitePieces, Bitboard blackPieces)
{
    return whitePieces | blackPieces;
}

Bitboard ComputeDiagonalMask(int square)
{
    Bitboard result = 0;
    int rank = square / 8, file = square % 8;

    // Traverse up-left and down-right along the diagonal
    for (int r = rank - 1, f = file - 1; r >= 0 && f >= 0; --r, --f)
        result |= (1ULL << (r * 8 + f));
    for (int r = rank + 1, f = file + 1; r < 8 && f < 8; ++r, ++f)
        result |= (1ULL << (r * 8 + f));

    return result;
}

Bitboard ComputeAntiDiagonalMask(int square)
{
    Bitboard result = 0;
    int rank = square / 8, file = square % 8;

    // Traverse up-right and down-left along the anti-diagonal
    for (int r = rank - 1, f = file + 1; r >= 0 && f < 8; --r, ++f)
        result |= (1ULL << (r * 8 + f));
    for (int r = rank + 1, f = file - 1; r < 8 && f >= 0; ++r, --f)
        result |= (1ULL << (r * 8 + f));

    return result;
}

void InitializeMasks()
{
    for (int square = 0; square < 64; ++square) {
        DIAGONAL_MASKS[square] = ComputeDiagonalMask(square);
        ANTI_DIAGONAL_MASKS[square] = ComputeAntiDiagonalMask(square);
    }
}

Bitboard DiagonalMask(int square)
{
    return DIAGONAL_MASKS[square];
}

Bitboard AntiDiagonalMask(int square)
{
    return ANTI_DIAGONAL_MASKS[square];
}

Bitboard BlockerMasks(Bitboard slidingPiece, Bitboard occupancy)
{
    int square = lsb(slidingPiece); // Find the least significant bit

    Bitboard diagonalMask = DiagonalMask(square);
    Bitboard antiDiagonalMask = AntiDiagonalMask(square);

    Bitboard diagonalBlockers = occupancy & diagonalMask;
    Bitboard antiDiagonalBlockers = occupancy & antiDiagonalMask;

    return diagonalBlockers | antiDiagonalBlockers;
}

