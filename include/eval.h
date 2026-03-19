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
    int material;
    int material_pst;     /* Material + piece-square tables (phase-interpolated) */
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

// @function material_eval
// @param board Board to evaluate.
// @returns int Material-only eval (centipawns).
int material_eval(Board* board);

// @function evaluation
// @param board Board to evaluate.
// @returns int PeSTO eval (centipawns, side-to-move).
int evaluation(Board* board);

// @function eval_breakdown
// @desc Fills *out with per-term breakdown; returns same value as pesto_eval.
// @param board Board to evaluate.
// @param out Output breakdown struct.
// @returns int PeSTO eval (centipawns).
int eval_breakdown(Board* board, EvalBreakdown* out);

// @function log_breakdown
// @desc Logs eval breakdown to stderr (e.g. for debugging).
// @param b EvalBreakdown
void log_breakdown(EvalBreakdown b);

// @function pesto_init
// @desc Initialize PeSTO tables. Call once at startup.
void pesto_init(void);

// @function pesto_set_tune_values
// @desc For Texel tuning: set mg_value[6], eg_value[6] and rebuild tables.
// @param mg_value Midgame piece values (pawn..king).
// @param eg_value Endgame piece values.
void pesto_set_tune_values(const int mg_value[6], const int eg_value[6]);

// @function pesto_material_pst_eval_white
// @desc Material + PST only, from white's perspective (centipawns).
// @param board Board to evaluate.
// @returns int Centipawns (white perspective).
int pesto_material_pst_eval_white(Board* board);

// @function nnue_eval
// @desc Evaluate position. Uses NNUE if loaded; otherwise returns 0 (caller should use PeSTO).
// @param board Board to evaluate.
// @returns int Centipawns or 0 if NNUE not loaded.
int nnue_eval(Board* board);

#endif // EVAL_H
