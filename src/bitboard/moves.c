#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include "move.h"

Bitboard DoMove(Bitboard* current, Move move)
{
    Square source, destination;
    uint8_t promotion, flag;
    MoveDecode(move, &source, &destination, &promotion, &flag);

    // Update the bitboard
    // Clear the source bit
    *current &= ~(1ULL << source);

    // Set the destination bit
    if(promotion == PROMOTION_NONE)
        *current |= (1ULL << destination);
    else {
        *current &= ~(1ULL << destination);
        // Returning the promotion square so it can be or'd with the appropriate bitboard 
        return 1ULL << destination; 
    }

    return 0;
}

Bitboard UndoMove(Bitboard* current, Move move)
{
    Square source, destination;
    uint8_t promotion, flag;
    MoveDecode(move, &source, &destination, &promotion, &flag);

    // Revert the move on the bitboard
    // Clear the destination bit
    *current &= ~(1ULL << destination);

    // Restore the source bit
    *current |= (1ULL << source);

    // Handle promotions
    if (promotion != PROMOTION_NONE) {
        *current &= ~(1ULL << source);
        return 1ULL << source;
    }

    return 0;
}

Bitboard GenerateWhitePawnMoves(Bitboard pieces, Bitboard king, Bitboard emptySquares, Bitboard enemySquares, Square enpassantSquare)
{
    Bitboard pseudoLegal = WhitePawnAttacks(pieces, enemySquares) 
        | WhitePawnPushes(pieces, emptySquares)
        | (WhitePawnAttacks(pieces, ~0ULL) & (1ULL << enpassantSquare));

    return pseudoLegal;
}

Bitboard GenerateBlackPawnMoves(Bitboard pieces, Bitboard king, Bitboard emptySquares, Bitboard enemySquares, Square enpassantSquare);
Bitboard GenerateKnightMoves(Bitboard pieces, Bitboard king, Bitboard emptySquares, Bitboard enemySquares);
Bitboard GenerateBishopMoves(Bitboard pieces, Bitboard king, Bitboard emptySquares, Bitboard enemySquares);
Bitboard GenerateRookMoves(Bitboard pieces, Bitboard king, Bitboard emptySquares, Bitboard enemySquares);
Bitboard GenerateQueenMoves(Bitboard pieces, Bitboard king, Bitboard emptySquares, Bitboard enemySquares);
Bitboard GenerateKingMoves(Bitboard pieces, Bitboard king, Bitboard emptySquares, Bitboard enemySquares);

bool IsKingInCheck(Bitboard kingPosition, Bitboard enemyAttacks)
{
    return kingPosition & enemyAttacks;
}
