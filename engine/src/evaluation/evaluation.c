#include "bitboard.h"
#include "board.h"
#include "evaluation.h"
#include "masks.h"
#include "movegen.h"
#include "heatmaps.h"
#include "piece.h"
#include "result.h"
#include "tuning.h"
#include <stdio.h>
#include <ctype.h>

void EvalPrint(Eval eval)
{
    printf("Total: %d\n", eval.total);
    printf("\tMaterial: %d\n", eval.material);
    printf("\tPiece Tables: %d\n", eval.piece_tables);
    printf("\tMobility: %d\n", eval.mobility);
    printf("\tPawn Structure: %d\n", eval.pawn_structure);
    printf("\tKing Safety: %d\n", eval.king_safety);
    printf("\tThreats: %d\n", eval.threats);
    printf("\tTempo Bonus: %d\n", eval.tempo_bonus);
    printf("\tBishop Pair Bonus: %d\n", eval.bishop_bonus);
}


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

    if(IsInsufficientMaterial(board)) return score;

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
    if(IsInsufficientMaterial(board)) return score;

    Bitboard kingBB = board->bitboards[color*6 + INDEX_KING];
    Bitboard opponent = GetEnemyColor(board, color);

    // TODO: enlarge the area below by one square
    Bitboard kingSurroundings = KingMoveMask(lsb(kingBB));
    Bitboard enemies = kingSurroundings & opponent;

    // Penalize king for being attacked
    score += -popcount(enemies);

    return score;
}

int EvaluateMobility(const Board* board, const Tuning* tuning, PieceColor color)
{
    if(IsInsufficientMaterial(board)) return 0;
    Board copy = *board;
    copy.turn = color;
    
    Moves list = GenerateMoves(&copy, MOVE_LEGAL);

    return list.count;
}

#define FILE_MASK(file) (0x0101010101010101ULL << (file))
int EvaluatePawnStructure(const Board* board, const Tuning* tuning, PieceColor color)
{
    int score = 0;
    Bitboard pawns = board->bitboards[color*6 + INDEX_BLACK_PAWN];

    while (pawns) {
        Square sq = poplsb(&pawns);
        int file = sq % 8;
        int rank = sq / 8;

        // Isolated pawn (no friendly pawns on adjacent files)
        Bitboard fileMask = FILE_MASK(file);
        Bitboard adjFiles = 0;
        if (file > 0) adjFiles |= FILE_MASK(file - 1);
        if (file < 7) adjFiles |= FILE_MASK(file + 1);
        Bitboard neighbors = board->bitboards[color*6 + INDEX_BLACK_PAWN] & adjFiles;
        if (neighbors == 0)
            score -= abs(tuning->isolatedPawnPenalty);

        // Doubled pawn (more than one pawn on the same file)
        Bitboard sameFile = board->bitboards[color*6 + INDEX_BLACK_PAWN] & fileMask;
        if (popcount(sameFile) > 1)
            score -= abs(tuning->doubledPawnPenalty);

        // Passed pawn (no opposing pawns in front or on adjacent files)
        Bitboard oppPawns = board->bitboards[!color*6 + INDEX_BLACK_PAWN];

        Bitboard passedMask = fileMask;
        if (file > 0)
            passedMask |= 0x0101010101010101ULL << (file - 1);
        if (file < 7)
            passedMask |= 0x0101010101010101ULL << (file + 1);

        if ((oppPawns & passedMask) == 0)
            score += tuning->passedPawnBonus[rank];
    }

    return score;
}

int EvaluateThreats(const Board* board, const Tuning* tuning, PieceColor color)
{
    int score = 0;

    // board->knights[color], board->bishops[color], etc.
    // and a function: Bitboard AttacksFrom(PieceType type, Square sq, const Board* board)

    // Loop through attackers (e.g., knights as an example)
    // Bitboard knights = board->knights[color];
    // while (knights) {
    //     Square from = poplsb(&knights);
    //     Bitboard attacks = KnightAttacks(from) & board->occupied[!color];
    //
    //     score += popcount(attacks) * tuning->knightThreatBonus;
    // }

    // You would do the same for other pieces like bishops, rooks, queens

    return score;
}
