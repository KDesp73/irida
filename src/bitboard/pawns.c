#include "bitboard.h"

Bitboard WhitePawnPushes(Bitboard pawns, Bitboard emptySquares)
{
    uint64_t oneSquarePushes = (pawns << 8) & emptySquares;
    uint64_t twoSquarePushes = ((pawns & RANK_2) << 16) & emptySquares;

    return oneSquarePushes | twoSquarePushes;
}

Bitboard BlackPawnPushes(Bitboard pawns, Bitboard emptySquares)
{
    uint64_t oneSquarePushes = (pawns >> 8) & emptySquares;
    uint64_t twoSquarePushes = ((pawns & RANK_7) >> 16) & emptySquares;

    return oneSquarePushes | twoSquarePushes;
}

Bitboard WhitePawnPromotions(Bitboard pawns, Bitboard emptySquares)
{
    return (pawns << 8) & emptySquares & RANK_8;
}

Bitboard BlackPawnPromotions(Bitboard pawns, Bitboard emptySquares)
{
    return (pawns >> 8) & emptySquares & RANK_1;
}

Bitboard BlackPawnPromotionCaptures(Bitboard pawns, Bitboard opponentPieces);
Bitboard WhitePawnPromotionCaptures(Bitboard pawns, Bitboard opponentPieces);
Bitboard WhitePawnPromotionCaptures(Bitboard pawns, Bitboard opponentPieces)
{
    // Capture to the left and promote
    Bitboard leftCapture = (pawns << 7) & opponentPieces & ~FILE_H & RANK_8;

    // Capture to the right and promote
    Bitboard rightCapture = (pawns << 9) & opponentPieces & ~FILE_A & RANK_8;

    return leftCapture | rightCapture;
}

Bitboard BlackPawnPromotionCaptures(Bitboard pawns, Bitboard opponentPieces)
{
    // Capture to the left and promote
    Bitboard leftCapture = (pawns >> 9) & opponentPieces & ~FILE_H & RANK_1;

    // Capture to the right and promote
    Bitboard rightCapture = (pawns >> 7) & opponentPieces & ~FILE_A & RANK_1;

    return leftCapture | rightCapture;
}
