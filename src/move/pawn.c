#include "board.h"
#include "move.h"
#include <chess/piece.h>
#include <stdlib.h>

_Bool CanMovePawn(const Board* board, Square from, Square to, Color color, Flag* flag)
{
    if (!IsSquareValid(from) || !IsSquareValid(to)) {
        return 0;
    }

    *flag = FLAG_NORMAL;

    int rank_from = Rank(from);
    int file_from = File(from);
    int rank_to = Rank(to);
    int file_to = File(to);

    int direction = (color == PIECE_COLOR_WHITE) ? 1 : -1;
    int starting_rank = (color == PIECE_COLOR_WHITE) ? 1 : 6;
    int promotion_rank = (color == PIECE_COLOR_WHITE) ? 7 : 0;

    // Regular move (1 square forward)
    if (file_from == file_to && rank_to == rank_from + direction) {
        if (IsSquareEmpty(board, to)) {
            if (rank_to == promotion_rank) {
                *flag = FLAG_PROMOTION;
            }
            return 1;
        }
    }

    // Double move (2 squares forward from starting position)
    if (file_from == file_to && rank_from == starting_rank && rank_to == rank_from + 2 * direction) {
        Square intermediate_square = from + direction * BOARD_SIZE;
        if (IsSquareEmpty(board, to) && IsSquareEmpty(board, intermediate_square)) {
            *flag = FLAG_PAWN_DOUBLE_MOVE;
            return 1;
        }
    }

    // Capture move (diagonal)
    if (abs(file_to - file_from) == 1 && rank_to == rank_from + direction) {
        if (IsSquareOccupiedBy(board, to, 1 - color)) {
            if (rank_to == promotion_rank) {
                *flag = FLAG_PROMOTION_WITH_CAPTURE;
            }
            return 1;
        }

        // En passant capture
        if (board->enpassant_square == to) {
            *flag = FLAG_ENPASSANT;
            return 1;
        }
    }

    return 0;
}

