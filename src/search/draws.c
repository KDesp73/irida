/*
 * Theory: Draw detection.
 *
 * A position is a draw if: (1) the 50-move rule applies (halfmove >= 100),
 * (2) the position has occurred three times (threefold repetition), or
 * (3) neither side has enough material to deliver checkmate (insufficient
 * material). The board and castro provide halfmove count and repetition
 * detection; we combine them here for the search to prune drawn positions.
 */
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
