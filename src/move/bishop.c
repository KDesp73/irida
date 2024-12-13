#include "move.h"
#include <stdint.h>
#include <stdlib.h>

_Bool CanMoveBishop(const Board* board, Square from, Square to, uint8_t color)
{
    if (!IsSquareValid(from) || !IsSquareValid(to)) {
        return 0;
    }

    int rank_diff = abs(Rank(to) - Rank(from));
    int file_diff = abs(File(to) - File(from));

    // Bishop moves diagonally, so rank difference must equal file difference
    if (rank_diff != file_diff) {
        return 0;
    }

    // Determine the direction of movement
    int rank_step = (Rank(to) > Rank(from)) ? 1 : -1;
    int file_step = (File(to) > File(from)) ? 1 : -1;

    // Check intermediate squares for obstruction
    int current_rank = Rank(from) + rank_step;
    int current_file = File(from) + file_step;
    while (current_rank != Rank(to) && current_file != File(to)) {
        Square current_square = current_rank * BOARD_SIZE + current_file;
        if (!IsSquareEmpty(board, current_square)) {
            return 0;
        }
        current_rank += rank_step;
        current_file += file_step;
    }

    // Check if the destination square is occupied by the same color
    if (IsSquareOccupiedBy(board, to, color)) {
        return 0;
    }

    return 1;
}
