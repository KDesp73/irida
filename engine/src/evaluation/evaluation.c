#include "bitboard.h"
#include "board.h"
#include "evaluation.h"
#include "masks.h"
#include "movegen.h"
#include "heatmaps.h"
#include "piece.h"
#include "tuning.h"
#include <ctype.h>

int EvaluateMaterial(const Board* board, const Tuning* tuning)
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
                value = THIS_OR(tuning->pieces.pawn, PAWN_VALUE);
                break;
            case 'n':
            case 'N':
                value = THIS_OR(tuning->pieces.knight, KNIGHT_VALUE);
                break;
            case 'b':
            case 'B':
                value = THIS_OR(tuning->pieces.bishop, BISHOP_VALUE);
                break;
            case 'r':
            case 'R':
                value = THIS_OR(tuning->pieces.rook, ROOK_VALUE);
                break;
            case 'q':
            case 'Q':
                value = THIS_OR(tuning->pieces.queen, QUEEN_VALUE);
                break;
            case 'k':
            case 'K':
                value = THIS_OR(tuning->pieces.king, KING_VALUE);
                break;
            }
            score += IS_WHITE(piece) ? value : -value;
        }
    }
    return score;
}

int EvaluatePieceSquareTables(const Board* board, const Tuning* tuning)
{
    int score = 0;

    for (int square = 0; square < 64; square++) {
        Piece piece = PieceAt(board, square);
        PieceColor color = piece.color;

        int val = 0;
        if (tolower(piece.type) == 'p') {
            val = PawnTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
        else if (tolower(piece.type) == 'n') {
            val = KnightTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
        else if (tolower(piece.type) == 'b') {
            val = BishopTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
        else if (tolower(piece.type) == 'r') {
            val = RookTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
        else if (tolower(piece.type) == 'q') {
            val = QueenTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
        else if (tolower(piece.type) == 'k') {
            val = KingTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
    }

    return score;
}

int EvaluateKingSafety(const Board* board, const Tuning* tuning)
{
    int score = 0;

    PieceColor color = board->turn;
    Bitboard kingBB = board->bitboards[color*6 + INDEX_KING];
    Bitboard opponent = GetEnemyColor(board, color);

    // TODO: enlarge the area below by one square
    Bitboard kingSurroundings = KingMoveMask(lsb(kingBB));
    Bitboard enemies = kingSurroundings & opponent;

    // Penalize king for being attacked
    score += -(popcount(enemies));

    return score;
}

int EvaluateMobility(const Board* board, const Tuning* tuning, PieceColor color)
{
    int mobility = 0;
    
    for (int square = 0; square < 64; square++) {
        Piece piece = PieceAt(board, square);
        if (piece.color == color) {
            mobility += GenerateMoves(board, MOVE_LEGAL).count;
        }
    }

    return mobility;
}

