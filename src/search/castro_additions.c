/* Search helpers not yet merged into castro.h (see TODO). */

#include "castro_additions.h"
#include "castro.h"
#include <stdbool.h>

#define FIFTY_MOVE_LIMIT 100


/* Draw for search pruning / repetition: 50-move, threefold, insufficient material. */
bool castro_IsDraw(Board* board)
{
    if (board->halfmove >= FIFTY_MOVE_LIMIT)
        return true;

    if (castro_IsThreefoldRepetition(board))
        return true;

    if (castro_IsInsufficientMaterial(board))
        return true;

    return false;
}

/*
 * True if side has knight, bishop, rook, or queen—used to gate null-move pruning
 * (NMP is unsafe in KPK etc. where zugzwang matters).
 */
bool castro_HasNonPawnMaterial(Board* board, int sideToMove)
{
    int start = (sideToMove == COLOR_WHITE) ? INDEX_WHITE_KNIGHT : INDEX_BLACK_KNIGHT;
    int end   = (sideToMove == COLOR_WHITE) ? INDEX_WHITE_KING   : INDEX_BLACK_KING;

    for (int i = start; i < end; i++) {
        if (board->bitboards[i] != 0ULL) return true;
    }
    return false;
}

size_t castro_PieceCount(Board* board)
{
    size_t count = 0;
    for(size_t i = 0; i < 12; ++i) {
        Bitboard bb = board->bitboards[i];
        count += popcount(bb);
    }
    return count;
}
