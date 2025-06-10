#ifndef ENGINE_EVALUATION_H
#define ENGINE_EVALUATION_H

#include "board.h"
#include "tuning.h"

typedef struct {
    int material;
    int mobility;
    int king_safety;
    int piece_tables;
    int tempo_bonus;
    int bishop_bonus;
    int total;
} Eval;

// https://www.chessprogramming.org/Evaluation

int EvaluateMaterial(const Board* board, const Tuning* tuning);
int EvaluatePieceSquareTables(const Board* board, const Tuning* tuning);
int EvaluateMobility(const Board* board, const Tuning* tuning, PieceColor color);
int EvaluateKingSafety(const Board* board, const Tuning* tuning, PieceColor color);

// TODO: EvaluatePawnStructure(const Board* board, const Tuning* tuning, PieceColor color);
// TODO: EvaluateThreats(const Board* board, const Tuning* tuning, PieceColor);


static inline Eval Evaluation(const Board* board)
{
    Tuning tuning = {0};
    LoadTuning(&tuning);

    Eval eval = {0};

    eval.material += EvaluateMaterial(board, &tuning);
    eval.piece_tables += EvaluatePieceSquareTables(board, &tuning);
    
    eval.king_safety += EvaluateKingSafety(board, &tuning, COLOR_WHITE);
    eval.king_safety -= EvaluateKingSafety(board, &tuning, COLOR_BLACK);

    eval.mobility += EvaluateMobility(board, &tuning, COLOR_WHITE);
    eval.mobility -= EvaluateMobility(board, &tuning, COLOR_BLACK);

    eval.tempo_bonus += (board->turn == COLOR_WHITE ? +10 : -10);

    if (popcount(board->bitboards[COLOR_WHITE * 6 + INDEX_BISHOP]) >= 2)
        eval.bishop_bonus += 30;
    if (popcount(board->bitboards[COLOR_BLACK * 6 + INDEX_BISHOP]) >= 2)
        eval.bishop_bonus -= 30;

    eval.total = 
        eval.material
        + eval.piece_tables
        + eval.king_safety
        + eval.mobility
        + eval.tempo_bonus
        + eval.bishop_bonus;

    return eval;
}

#endif // ENGINE_EVALUATION_H
