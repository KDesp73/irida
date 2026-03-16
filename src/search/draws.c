#include "draws.h"
#include "castro.h"
#include <stdbool.h>

#define FIFTY_MOVE_LIMIT 100


bool is_draw(Board* board)
{
    if (board->halfmove >= FIFTY_MOVE_LIMIT)
        return true;

    if (castro_IsThreefoldRepetition(board))
        return true;

    if (castro_IsInsufficientMaterial(board))
        return true;

    return false;
}
