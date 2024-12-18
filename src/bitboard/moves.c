#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include "move.h"
#include "generator.h"
#include "square.h"
#include <io/logging.h>

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


bool IsKingInCheck(Bitboard kingPosition, Bitboard enemyAttacks)
{
    return kingPosition & enemyAttacks;
}

bool IsSquareAttacked(const Board* board, Square square, Color attackerColor)
{
    Bitboard attacks = GeneratePseudoLegalAttacks(board, attackerColor);
    return attacks & (1ULL << square);
}
