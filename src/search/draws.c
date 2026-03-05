#include "draws.h"
#include "castro.h"
#include <stdbool.h>

#define FIFTY_MOVE_LIMIT 100

static bool insufficient_material(const Board* board)
{
    int wq = castro_CountPieces(board, COLOR_WHITE, QUEEN);
    int wr = castro_CountPieces(board, COLOR_WHITE, ROOK);
    int wp = castro_CountPieces(board, COLOR_WHITE, PAWN);
    int wn = castro_CountPieces(board, COLOR_WHITE, KNIGHT);
    int wb = castro_CountPieces(board, COLOR_WHITE, BISHOP);

    int bq = castro_CountPieces(board, COLOR_BLACK, QUEEN);
    int br = castro_CountPieces(board, COLOR_BLACK, ROOK);
    int bp = castro_CountPieces(board, COLOR_BLACK, PAWN);
    int bn = castro_CountPieces(board, COLOR_BLACK, KNIGHT);
    int bb = castro_CountPieces(board, COLOR_BLACK, BISHOP);

    if (wp > 0 || bp > 0 || wq > 0 || bq > 0 || wr > 0 || br > 0)
        return false;

    int w_minors = wn + wb;
    int b_minors = bn + bb;

    if (w_minors == 0 && b_minors == 0)
        return true; /* K vs K */

    if (w_minors == 0 && b_minors == 1)
        return true; /* K vs K+N or K vs K+B */
    if (w_minors == 1 && b_minors == 0)
        return true;

    /* K+B vs K+B with same-color bishops only */
    if (w_minors == 1 && b_minors == 1 && wb == 1 && bb == 1) {
        /* Find one white and one black bishop, check same color square */
        Square ws = SQUARE_NONE, bs = SQUARE_NONE;
        for (int r = 0; r < 8 && (ws == SQUARE_NONE || bs == SQUARE_NONE); r++) {
            for (int f = 0; f < 8; f++) {
                char pc = board->grid[r][f];
                if (pc == 'B') ws = castro_SquareFromCoords((size_t)r, (size_t)f);
                if (pc == 'b') bs = castro_SquareFromCoords((size_t)r, (size_t)f);
            }
        }
        if (ws != SQUARE_NONE && bs != SQUARE_NONE) {
            int w_sq_color = (castro_Rank(ws) + castro_File(ws)) % 2;
            int b_sq_color = (castro_Rank(bs) + castro_File(bs)) % 2;
            if (w_sq_color == b_sq_color)
                return true;
        }
    }

    return false;
}

bool is_draw(Board* board)
{
    if (board->halfmove >= FIFTY_MOVE_LIMIT)
        return true;

    if (castro_IsThreefoldRepetition(board))
        return true;

    if (insufficient_material(board))
        return true;

    return false;
}
