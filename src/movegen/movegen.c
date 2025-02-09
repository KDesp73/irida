#include "movegen.h"

// TODO: write the rest of the functions
Moves GenerateMoves(const Board* board, MoveType type)
{
    switch (type) {
    case MOVE_LEGAL:
        return GenerateLegalMoves(board);
    case MOVE_PSEUDO:
        return GeneratePseudoLegalMoves(board);
    case MOVE_CAPTURES:
    case MOVE_QUIET:
    case MOVE_CHECK:
    case MOVE_CHECKMATE:
    case MOVE_CASTLING:
    case MOVE_EN_PASSANT:
    case MOVE_PROMOTION:
    case MOVE_ILLEGAL:
        break;
    }

    return (Moves){.count = 0};
}

