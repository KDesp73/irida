#include "move.h"
#include <stdlib.h>
#include <chess/piece.h>

_Bool CanMoveKing(const Board* board, Square from, Square to, Color color, Flag* flag)
{
    if (!IsSquareValid(from) || !IsSquareValid(to)) {
        return 0;
    }

    int rank_from = Rank(from);
    int file_from = File(from);
    int rank_to = Rank(to);
    int file_to = File(to);

    int dr = abs(rank_to - rank_from);
    int df = abs(file_to - file_from);

    if (dr <= 1 && df <= 1) {
        if (!IsSquareOccupiedBy(board, to, color)) {
            return 1;
        }
    }

    // Castling
    if (dr == 0 && df == 2) { 
        if (color == PIECE_COLOR_WHITE) {
            if ((to > from) && (board->state.castling_rights & (1 << 0))) { // White kingside castling
                // Additional logic to check path clear and no checks on squares
                return 1; // Replace with actual castling validation
            } else if ((to < from) && (board->state.castling_rights & (1 << 1))) { // White queenside castling
                return 1; // Replace with actual castling validation
            }
        } else if (color == PIECE_COLOR_BLACK) {
            if ((to > from) && (board->state.castling_rights & (1 << 2))) { // Black kingside castling
                return 1; // Replace with actual castling validation
            } else if ((to < from) && (board->state.castling_rights & (1 << 3))) { // Black queenside castling
                return 1; // Replace with actual castling validation
            }
        }
    }

    return 0;
}


