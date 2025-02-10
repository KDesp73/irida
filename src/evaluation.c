#include "board.h"
#include "evaluation.h"
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

        if (tolower(piece.type) == 'p') {
            int val = PawnTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
        else if (tolower(piece.type) == 'n') {
            int val = KnightTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
        else if (tolower(piece.type) == 'b') {
            int val = BishopTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
        else if (tolower(piece.type) == 'r') {
            int val = RookTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
        else if (tolower(piece.type) == 'q') {
            int val = QueenTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
        else if (tolower(piece.type) == 'k') {
            int val = KingTableValue(board, tuning, square);
            score += (color == COLOR_WHITE) ? val : -val;
        }
    }

    return score;
}

int EvaluateKingSafety(const Board* board, const Tuning* tuning)
{
    int score = 0;

    // // Pseudo-code
    // Square kingSquare = GetKingPosition(board);
    // 
    // // Check surrounding squares for opponent pieces
    // for (int square = 0; square < 8; square++) {
    //     if (IsOpponentPiece(board, square, GetPieceColorAt(board, kingSquare))) {
    //         score -= 1;  // Penalize king for being attacked
    //     }
    // }

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

