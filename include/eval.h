#ifndef EVAL_H
#define EVAL_H

#include "castro.h"

// @module eval
// @desc Evaluation: EvalFn type, PeSTO (material+PST, terms), NNUE wrapper, breakdown.

// @type EvalFn
// @desc Function pointer: Board* -> int (centipawns, side-to-move perspective).
typedef int (*EvalFn)(Board*) ;

// @struct EvalBreakdown
// @desc Per-term breakdown for PeSTO eval (centipawns, side-to-move). total equals pesto_eval when convention matches.
typedef struct EvalBreakdown {
    int material;         /* Always 0: piece values are included in material_pst (PeSTO); see irida_EvalMaterial for raw material */
    int material_pst;     /* PeSTO material + piece-square tables (phase-interpolated) */
    int pawn_structure;
    int mobility;
    int king_safety;
    int piece_activity;
    int space;
    int threats;
    int endgame;
    int total;            /* Sum of above, equals pesto_eval() when side-to-move convention matches */
    int game_phase;       /* 0..24, for context */
} EvalBreakdown;

// @function EvalMaterialWhiteMinusBlack
// @desc Sum(white piece values) − sum(black). Debug / material-only eval; not added to irida_Evaluation (PeSTO carries material in PST).
int irida_EvalMaterialWhiteMinusBlack(Board* board);

// @function EvalMaterial
// @desc Material-only score in centipawns from the side-to-move perspective.
int irida_EvalMaterial(Board* board);

// @function Evaluation
// @param board Board to evaluate.
// @returns int PeSTO eval (centipawns, side-to-move).
int irida_Evaluation(Board* board);

// @function EvalBreakdown
// @desc Fills *out with per-term breakdown; returns same value as pesto_eval.
// @param board Board to evaluate.
// @param out Output breakdown struct.
// @returns int PeSTO eval (centipawns).
int irida_EvalBreakdown(Board* board, EvalBreakdown* out);

// @function EvalBreakdownLog 
// @desc Logs eval breakdown to stderr (e.g. for debugging).
// @param b EvalBreakdown
void irida_EvalBreakdownLog(EvalBreakdown b);

// @function EvalPestoInit
// @desc Initialize PeSTO tables. Call once at startup.
void irida_EvalPestoInit(void);

// @function EvalPestoSetTuneValues
// @desc For Texel tuning: set mg_value[6], eg_value[6] and rebuild tables.
// @param mg_value Midgame piece values (pawn..king).
// @param eg_value Endgame piece values.
void irida_EvalPestoSetTuneValues(const int mg_value[6], const int eg_value[6]);

// @function EvalPestoMaterialPstEvalWhite
// @desc Material + PST only, from white's perspective (centipawns).
// @param board Board to evaluate.
// @returns int Centipawns (white perspective).
int irida_EvalPestoMaterialPstEvalWhite(Board* board);

// @function EvalNNUE
// @desc Evaluate position. Uses NNUE if loaded; otherwise returns 0 (caller should use PeSTO).
// @param board Board to evaluate.
// @returns int Centipawns or 0 if NNUE not loaded.
int irida_EvalNNUE(Board* board);

#endif // EVAL_H
