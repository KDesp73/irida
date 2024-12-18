#include "piece.h"
#include "board.h"

Piece PieceAt(const Board* board, Square square)
{
    static const Piece pieceLookup[PIECE_TYPES] = {
        [INDEX_BLACK_KING]   = {.type = 'k', .color = COLOR_BLACK},
        [INDEX_BLACK_QUEEN]  = {.type = 'q', .color = COLOR_BLACK},
        [INDEX_BLACK_ROOK]   = {.type = 'r', .color = COLOR_BLACK},
        [INDEX_BLACK_BISHOP] = {.type = 'b', .color = COLOR_BLACK},
        [INDEX_BLACK_KNIGHT] = {.type = 'n', .color = COLOR_BLACK},
        [INDEX_BLACK_PAWN]   = {.type = 'p', .color = COLOR_BLACK},
        [INDEX_WHITE_KING]   = {.type = 'K', .color = COLOR_WHITE},
        [INDEX_WHITE_QUEEN]  = {.type = 'Q', .color = COLOR_WHITE},
        [INDEX_WHITE_ROOK]   = {.type = 'R', .color = COLOR_WHITE},
        [INDEX_WHITE_BISHOP] = {.type = 'B', .color = COLOR_WHITE},
        [INDEX_WHITE_KNIGHT] = {.type = 'N', .color = COLOR_WHITE},
        [INDEX_WHITE_PAWN]   = {.type = 'P', .color = COLOR_WHITE}
    };

    for (size_t i = 0; i < PIECE_TYPES; i++) {
        if (board->bitboards[i] & (1ULL << square)) {
            return pieceLookup[i];
        }
    }

    return (Piece){.type = ' ', .color = COLOR_NONE};
}

