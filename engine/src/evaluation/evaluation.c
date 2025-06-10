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

    for (int color = 0; color < 2; color++) {
        for (int type = 0; type < 6; type++) {
            Bitboard pieces = board->bitboards[color * 6 + type];

            double value = 0;
            switch (type) {
                case INDEX_PAWN:   value = THIS_OR(tuning->pieces.pawn, PAWN_VALUE); break;
                case INDEX_KNIGHT: value = THIS_OR(tuning->pieces.knight, KNIGHT_VALUE); break;
                case INDEX_BISHOP: value = THIS_OR(tuning->pieces.bishop, BISHOP_VALUE); break;
                case INDEX_ROOK:   value = THIS_OR(tuning->pieces.rook, ROOK_VALUE); break;
                case INDEX_QUEEN:  value = THIS_OR(tuning->pieces.queen, QUEEN_VALUE); break;
                case INDEX_KING:   value = THIS_OR(tuning->pieces.king, KING_VALUE); break;
            }

            while (pieces) {
                poplsb(&pieces);
                score += (color == COLOR_WHITE) ? value : -value;
            }
        }
    }

    return score;
}

int EvaluatePieceSquareTables(const Board* board, const Tuning* tuning)
{
    int score = 0;

    for (int square = 0; square < 64; square++) {
        Piece piece = PieceAt(board, square);
        if (piece.type == 0) continue;

        bool isWhite = piece.color == COLOR_WHITE;
        int val = 0;

        switch (tolower(piece.type)) {
            case 'p': val = PawnTableValue(board, tuning, square, isWhite); break;
            case 'n': val = KnightTableValue(board, tuning, square, isWhite); break;
            case 'b': val = BishopTableValue(board, tuning, square, isWhite); break;
            case 'r': val = RookTableValue(board, tuning, square, isWhite); break;
            case 'q': val = QueenTableValue(board, tuning, square, isWhite); break;
            case 'k': val = KingTableValue(board, tuning, square, isWhite); break;
        }

        score += isWhite ? val : -val;
    }

    return score;
}

int EvaluateKingSafety(const Board* board, const Tuning* tuning, PieceColor color)
{
    int score = 0;

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
    Board copy = *board;
    copy.turn = color;
    
    Moves list = GenerateMoves(&copy, MOVE_LEGAL);

    return list.count;
}

