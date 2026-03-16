// TODO: add to castro.h

#include "castro_additions.h"
#include "castro.h"
#include <stdbool.h>

#define FIFTY_MOVE_LIMIT 100


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

bool castro_HasNonPawnMaterial(Board* board)
{
    for(size_t i = INDEX_KNIGHT; i < INDEX_KING; i++) {
        if(board->bitboards[i] != 0ULL) return true;
    }
    return false;
}
