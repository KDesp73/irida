#include "piece.h"
#include "board.h"
#include <chess/piece.h>

Piece PieceAt(const Board* board, Square square)
{
    static const Piece pieceLookup[PIECE_TYPES] = {
        [INDEX_BLACK_KING]   = {.type = 'k', .color = PIECE_COLOR_BLACK},
        [INDEX_BLACK_QUEEN]  = {.type = 'q', .color = PIECE_COLOR_BLACK},
        [INDEX_BLACK_ROOK]   = {.type = 'r', .color = PIECE_COLOR_BLACK},
        [INDEX_BLACK_BISHOP] = {.type = 'b', .color = PIECE_COLOR_BLACK},
        [INDEX_BLACK_KNIGHT] = {.type = 'n', .color = PIECE_COLOR_BLACK},
        [INDEX_BLACK_PAWN]   = {.type = 'p', .color = PIECE_COLOR_BLACK},
        [INDEX_WHITE_KING]   = {.type = 'K', .color = PIECE_COLOR_WHITE},
        [INDEX_WHITE_QUEEN]  = {.type = 'Q', .color = PIECE_COLOR_WHITE},
        [INDEX_WHITE_ROOK]   = {.type = 'R', .color = PIECE_COLOR_WHITE},
        [INDEX_WHITE_BISHOP] = {.type = 'B', .color = PIECE_COLOR_WHITE},
        [INDEX_WHITE_KNIGHT] = {.type = 'N', .color = PIECE_COLOR_WHITE},
        [INDEX_WHITE_PAWN]   = {.type = 'P', .color = PIECE_COLOR_WHITE}
    };

    for (size_t i = 0; i < PIECE_TYPES; i++) {
        if (board->bitboards[i] & (1ULL << square)) {
            return pieceLookup[i];
        }
    }

    return (Piece){.type = ' ', .color = PIECE_COLOR_NONE};
}

