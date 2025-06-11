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
    int pawn_structure;
    int threats;
    int total;
} Eval;

void EvalPrint(Eval eval);

// https://www.chessprogramming.org/Evaluation

int ComputeGamePhase(const Board* board, const Tuning* tuning);

int EvaluateMaterial(const Board* board, const Tuning* tuning);
int EvaluatePieceSquareTables(const Board* board, const Tuning* tuning);
int EvaluateMobility(const Board* board, const Tuning* tuning, PieceColor color);
int EvaluateKingSafety(const Board* board, const Tuning* tuning, PieceColor color);
int EvaluatePawnStructure(const Board* board, const Tuning* tuning, PieceColor color);

// TODO: 
int EvaluateThreats(const Board* board, const Tuning* tuning, PieceColor);

// TODO: 
// 1. Evaluate checkmate threats
// 2. Evaluate forced checkmates
// 3. Insentivize checks and force the king to the edge of the board


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

    // eval.tempo_bonus += (board->turn == COLOR_WHITE ? +tuning.tempoBonus : -tuning.tempoBonus);

    if (popcount(board->bitboards[COLOR_WHITE * 6 + INDEX_BISHOP]) >= 2)
        eval.bishop_bonus += tuning.bishopPairBonus;
    if (popcount(board->bitboards[COLOR_BLACK * 6 + INDEX_BISHOP]) >= 2)
        eval.bishop_bonus -= tuning.bishopPairBonus;

    eval.pawn_structure += EvaluatePawnStructure(board, &tuning, COLOR_WHITE);
    eval.pawn_structure -= EvaluatePawnStructure(board, &tuning, COLOR_BLACK);

    eval.total = eval.material
        + eval.piece_tables
        + eval.king_safety
        + eval.mobility
        + eval.tempo_bonus
        + eval.bishop_bonus
        + eval.pawn_structure
        ;

    return eval;
}

#endif // ENGINE_EVALUATION_H
