#include "evaluation.h"
#include "board.h"
#include "piece.h"

int EvaluateMaterial(const Board* board)
{
    int score = 0;
    for (size_t i = 0; i < PIECE_TYPES; i++) {
        Bitboard pieces = board->bitboards[i];
        while (pieces) {
            Square square = poplsb(&pieces);
            Piece piece = PieceAt(board, square);
            int value = 0;
            switch (piece.type) {
            case 'p':
            case 'P':
                value = PAWN_VALUE;
                break;
            case 'n':
            case 'N':
                value = KNIGHT_VALUE;
                break;
            case 'b':
            case 'B':
                value = BISHOP_VALUE;
                break;
            case 'r':
            case 'R':
                value = ROOK_VALUE;
                break;
            case 'q':
            case 'Q':
                value = QUEEN_VALUE;
                break;
            case 'k':
            case 'K':
                value = KING_VALUE;
                break;
            }
            score += IS_WHITE(piece) ? value : -value;
        }
    }
    return score;
}

