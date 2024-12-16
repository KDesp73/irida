#include "bitboard.h"
#include "square.h"
#include <io/logging.h>

Bitboard WhitePawnPushes(Bitboard pawns, Bitboard emptySquares)
{
    Bitboard oneSquarePushes = (pawns << 8) & emptySquares;
    Bitboard twoSquarePushes = (((pawns & RANK_2) << 8) & emptySquares) << 8 & emptySquares;

    return oneSquarePushes | twoSquarePushes;
}

Bitboard BlackPawnPushes(Bitboard pawns, Bitboard emptySquares)
{
    Bitboard oneSquarePushes = (pawns >> 8) & emptySquares;
    Bitboard twoSquarePushes = (((pawns & RANK_7) >> 8) & emptySquares) >> 8 & emptySquares;

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

Bitboard WhitePawnAttacks(Bitboard pawns, Bitboard enemySquares)
{
    uint64_t left_attacks = (pawns & ~FILE_A) << 7;
    uint64_t right_attacks = (pawns & ~FILE_H) << 9;

    left_attacks &= enemySquares;
    right_attacks &= enemySquares;

    return left_attacks | right_attacks;
}

Bitboard BlackPawnAttacks(Bitboard pawns, Bitboard enemySquares)
{
    uint64_t left_attacks = (pawns & ~FILE_H) >> 7;
    uint64_t right_attacks = (pawns & ~FILE_A) >> 9;

    left_attacks &= enemySquares;
    right_attacks &= enemySquares;

    return left_attacks | right_attacks;
}

Bitboard KnightAttacks(Bitboard knights, Bitboard emptySquares, Bitboard enemySquares)
{
    Bitboard emptyOrEnemySquares = emptySquares | enemySquares;
    return (((knights << 6) & ~FILE_H & ~FILE_G)
         | ((knights << 10) & ~FILE_A & ~FILE_B)
         | ((knights >> 6) & ~FILE_A & ~FILE_B)
         | ((knights >> 10) & ~FILE_H & ~FILE_G)
         | ((knights << 15)  & ~FILE_H)
         | ((knights << 17) & ~FILE_A)
         | ((knights >> 15) & ~FILE_A)
         | ((knights >> 17) & ~FILE_H))
         & emptyOrEnemySquares; 
}

Bitboard KingAttacks(Bitboard king, Bitboard emptySquares, Bitboard enemySquares)
{
    Bitboard emptyOrEnemySquares = emptySquares | enemySquares;
    return (((king << 1) & ~FILE_A)
         | ((king << 7) & ~FILE_H)
         | ((king << 8))
         | ((king << 9) & ~FILE_A)
         | ((king >> 1) & ~FILE_H)
         | ((king >> 7) & ~FILE_A)
         | ((king >> 8))
         | ((king >> 9) & ~FILE_H))
         & emptyOrEnemySquares;
}

Bitboard generateDiagonal(Square square, int direction, Bitboard emptySquares, Bitboard enemySquares)
{
    Bitboard emptyOrEnemySquares = emptySquares | enemySquares;
    Bitboard friendlyPieces = ~emptyOrEnemySquares;
    Bitboard attacks = 0ULL;
    Bitboard current = 1ULL << square; // Start from the current square

    while (true) {
        // Move to the next square in the given direction
        if (direction > 0) {
            current <<= direction;
        } else {
            current >>= -direction;
        }

        BitboardPrint(current);

        // Break if the shift wraps around the board
        if ((direction == 9 || direction == -7) && (current & FILE_A)) { DEBU("Out of bounds");break; } // Wraps to FILE_A
        if ((direction == 7 || direction == -9) && (current & FILE_H)) { DEBU("Out of bounds");break; } // Wraps to FILE_H
        if (!current) { DEBU("Out of bounds");break; } // Outside the board

        // Check if the square is occupied by a friendly piece (stop there)
        if (friendlyPieces & current) { DEBU("Friendly piece");break; }

        // Add the square to attacks only if it is an empty or enemy piece
        attacks |= current;

        // Stop if it is an enemy piece (capture)
        if (enemySquares & current) { DEBU("Enemy piece");break; }
    }

    return attacks;
}


Bitboard generateBishopDiagonals(Square square, Bitboard emptySquares, Bitboard enemySquares)
{
    Bitboard attacks = 0ULL;

    attacks |= generateDiagonal(square, NORTH_EAST, emptySquares, enemySquares);
    attacks |= generateDiagonal(square, NORTH_WEST, emptySquares, enemySquares);
    attacks |= generateDiagonal(square, SOUTH_WEST, emptySquares, enemySquares);
    attacks |= generateDiagonal(square, SOUTH_EAST, emptySquares, enemySquares);

    return attacks;
}
Bitboard BishopAttacks(Bitboard bishops, Bitboard emptySquares, Bitboard enemySquares)
{
    Bitboard attacks = 0ULL;

    while (bishops) {
        Square bishopSquare = poplsb(&bishops);

        attacks |= generateBishopDiagonals(bishopSquare, emptySquares, enemySquares);
    }

    return attacks;
}


Bitboard generateLine(Square square, int direction, Bitboard emptySquares, Bitboard enemySquares)
{
    Bitboard emptyOrEnemySquares = emptySquares | enemySquares;
    Bitboard friendlyPieces = ~emptyOrEnemySquares;
    Bitboard attacks = 0ULL;
    Bitboard current = 1ULL << square; // Start from the current square

    while (true) {
        // Move to the next square in the given direction
        if (direction > 0) {
            current <<= direction;
        } else {
            current >>= -direction;
        }

        // Break if we go out of bounds or wrap around
        if ((direction == 1) && (current & FILE_A)) break;
        if ((direction == -1) && (current & FILE_H)) break;
        if ((direction == 8) && (current & RANK_1)) break;
        if ((direction == 8) && (current & RANK_1)) break; 
        if ((direction == -8) && (current & RANK_8)) break; 
        if (!current) break; // Out of bounds

        // Check if the square is occupied by a friendly piece (stop there)
        if (friendlyPieces & current) {
            break; // Stop at friendly pieces
        }

        // Add the square to attacks only if it is an empty or enemy piece
        attacks |= current;

        // Stop if it is an enemy piece (capture)
        if (enemySquares & current) {
            break; // Stop and capture enemy piece
        }
    }

    return attacks;
}

Bitboard generateRookLines(Square square, Bitboard emptySquares, Bitboard enemySquares)
{
    Bitboard attacks = 0ULL;

    attacks |= generateLine(square, NORTH, emptySquares, enemySquares);
    attacks |= generateLine(square, SOUTH, emptySquares, enemySquares);
    attacks |= generateLine(square, EAST, emptySquares, enemySquares);
    attacks |= generateLine(square, WEST, emptySquares, enemySquares);

    return attacks;
}

Bitboard RookAttacks(Bitboard rooks, Bitboard emptySquares, Bitboard enemySquares)
{
    Bitboard attacks = 0ULL;

    // Process each rook on the board
    while (rooks) {
        Square rookSquare = poplsb(&rooks);  // Get the rook's square

        attacks |= generateRookLines(rookSquare, emptySquares, enemySquares);  // Add rook's attacks
    }

    return attacks;
}

Bitboard QueenAttacks(Bitboard queens, Bitboard emptySquares, Bitboard enemySquares)
{
    return BishopAttacks(queens, emptySquares, enemySquares) | RookAttacks(queens, emptySquares, enemySquares);
}
