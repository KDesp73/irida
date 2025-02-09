#include "evaluation.h"
#include "board.h"
#include "movegen.h"
#include "piece-tables.h"
#include "piece.h"
#include <ctype.h>

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

int EvaluatePieceSquareTables(const Board* board)
{
    int score = 0;

    for (int square = 0; square < 64; square++) {
        Piece piece = PieceAt(board, square);
        PieceColor color = piece.color;

        if (tolower(piece.type) == 'p') {
            score += (color == COLOR_WHITE) ? PawnTable[square] : -PawnTable[square];
        }
        else if (tolower(piece.type) == 'n') {
            score += (color == COLOR_WHITE) ? KnightTable[square] : -KnightTable[square];
        }
        else if (tolower(piece.type) == 'b') {
            score += (color == COLOR_WHITE) ? BishopTable[square] : -BishopTable[square];
        }
        else if (tolower(piece.type) == 'r') {
            score += (color == COLOR_WHITE) ? RookTable[square] : -RookTable[square];
        }
        else if (tolower(piece.type) == 'q') {
            score += (color == COLOR_WHITE) ? QueenTable[square] : -QueenTable[square];
        }
        else if (tolower(piece.type) == 'k') {
            score += (color == COLOR_WHITE) ? KingTable[square] : -KingTable[square];
        }
    }

    return score;
}

int EvaluateKingSafety(const Board* board) 
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

int EvaluateMobility(const Board* board, PieceColor color)
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

