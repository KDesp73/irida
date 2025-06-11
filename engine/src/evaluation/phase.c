#include "board.h"
#include "tuning.h"

#define MAX_PHASE 24 // 1+1+2+4 per side = 8 × 2 = 16 (knights, bishops, rooks, queens)

int ComputeGamePhase(const Board* board, const Tuning* tuning)
{
    int phase = 0;

    for (PieceType pt = PAWN; pt <= KING; ++pt) {
        int countWhite = CountPieces(board, COLOR_WHITE, pt);
        int countBlack = CountPieces(board, COLOR_BLACK, pt);
        int weight = tuning->phaseWeight[pt];
        phase += weight * (countWhite + countBlack);
    }

    if (phase > MAX_PHASE) phase = MAX_PHASE;
    return phase;
}
