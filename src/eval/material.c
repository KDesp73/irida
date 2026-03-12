/*
 * Theory: Material-only evaluation.
 *
 * The score is the difference in piece values (white total minus black total)
 * using a fixed value per piece type (pawn, knight, bishop, rook, queen; king
 * not counted). Used as a simple fallback or for debugging; the main eval is
 * PeSTO or NNUE.
 */
#include "castro.h"
#include "eval.h"

int material_eval(Board* board)
{
    size_t white = 0, black = 0;

    const int piece_values[12] = {
        100,
        320,
        330,
        500,
        900,
        0,  
        100,
        320,
        330,
        500,
        900,
        0   
    };

    for (int i = INDEX_WHITE_PAWN; i <= INDEX_WHITE_QUEEN; i++) {
        uint64_t bb = board->bitboards[i];
        while (bb) {
            poplsb(&bb);
            white += piece_values[i];
        }
    }

    for (int i = INDEX_BLACK_PAWN; i <= INDEX_BLACK_QUEEN; i++) {
        uint64_t bb = board->bitboards[i];
        while (bb) {
            poplsb(&bb);
            black += piece_values[i];
        }
    }

    return (int)(white - black);
}
