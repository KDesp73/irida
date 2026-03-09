#ifndef EVAL_H
#define EVAL_H

#include "castro.h"

typedef int (*EvalFn)(Board*) ;

/* Per-term breakdown for PeSTO eval (all in centipawns, side-to-move perspective). */
typedef struct EvalBreakdown {
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

int material_eval(Board* board);
int pesto_eval(Board* board);
/* Fills *out with per-term breakdown and returns same value as pesto_eval. */
int pesto_eval_breakdown(Board* board, EvalBreakdown* out);
/* Logs eval breakdown to stderr (e.g. for debugging). */
void pesto_log_breakdown(Board* board);
void pesto_init(void);
/* For Texel tuning: set mg_value[6], eg_value[6] and rebuild tables; then pesto_material_pst_eval_white uses them. */
void pesto_set_tune_values(const int mg_value[6], const int eg_value[6]);
/* Material + PST only, from white's perspective (centipawns). */
int pesto_material_pst_eval_white(Board* board);

/* Evaluate position. Uses NNUE if loaded, otherwise returns 0 (caller should use PeSTO). */
int nnue_eval(Board* board);

#endif // EVAL_H
