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
    *current |= (1ULL << destination);

    if(flag == FLAG_PROMOTION){
        *current &= ~(1ULL << destination);
        return 1ULL << destination; 
    }

    return 0;
}

Bitboard UndoMove(Bitboard* current, Move move)
{
    MOVE_DECODE(move);

    *current &= ~(1ULL << dst);

    *current |= (1ULL << src);

    if (flag == FLAG_PROMOTION) {
        *current &= ~(1ULL << src);
        return 1ULL << dst;
    }
    if(flag == FLAG_CASTLING) {
        if(File(dst) > File(src))
            return 1ULL << (dst+1) | 1ULL << (dst-1);
        else 
            return 1ULL << (dst+1) | 1ULL << (dst-2);
    }
    if(flag == FLAG_ENPASSANT) {
        int color = (Rank(dst) > Rank(src));
        return 1ULL << (dst + ((color) ? -8 : 8));
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
