#include "move.h"

_Bool CanMoveRook(const Board* board, Square from, Square to, uint8_t color) {
    if (!IsSquareValid(from) || !IsSquareValid(to)) {
        return 0;
    }

    int rank_from = Rank(from);
    int file_from = File(from);
    int rank_to = Rank(to);
    int file_to = File(to);

    // Ensure it's a straight line move
    if (rank_from == rank_to || file_from == file_to) {
        int rank_step = (rank_to > rank_from) ? 1 : (rank_to < rank_from) ? -1 : 0;
        int file_step = (file_to > file_from) ? 1 : (file_to < file_from) ? -1 : 0;

        Square current = from;
        while (current != to) {
            current += rank_step * BOARD_SIZE + file_step;

            if (current == to) {
                break;
            }

            if (!IsSquareEmpty(board, current)) {
                return 0;
            }
        }

        // Check the destination square
        if (!IsSquareOccupiedBy(board, to, color)) {
            return 1;
        }
    }

    return 0;
}
