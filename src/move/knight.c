#include "move.h"
#include <stdlib.h>

_Bool CanMoveKnight(const Board* board, Square from, Square to, uint8_t color)
{
    if (!IsSquareValid(from) || !IsSquareValid(to)) {
        return 0;
    }

    int rank_from = Rank(from);
    int file_from = File(from);
    int rank_to = Rank(to);
    int file_to = File(to);

    int rank_diff = abs(rank_to - rank_from);
    int file_diff = abs(file_to - file_from);

    // Check if the move follows the "L" pattern of a knight
    if (!((rank_diff == 2 && file_diff == 1) || (rank_diff == 1 && file_diff == 2))) {
        return 0;
    }

    // Check if the destination square is occupied by the same color
    if (IsSquareOccupiedBy(board, to, color)) {
        return 0;
    }

    return 1;
}
