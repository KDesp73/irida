#ifndef TT_H
#define TT_H

#include "castro.h"

// @module tt
// @desc Transposition table: entry type, init/clear, probe/store.

// @enum TTNodeType
// @desc TT score type: exact, lower bound, upper bound, or none.
typedef enum {
    TT_EXACT,
    TT_LOWERBOUND,
    TT_UPPERBOUND,
    TT_NONE
} TTNodeType;

// @struct TTEntry
// @desc Single TT entry: key, depth, score, type, best move, generation.
typedef struct {
    uint64_t key;
    int depth;
    int score;
    TTNodeType type;
    Move bestMove;
    uint16_t generation;  /* search generation: only use entry if matches current */
} TTEntry;

// @function tt_init
// @param mb Size in megabytes.
void irida_TTInit(size_t mb);

// @function tt_clear
// @desc Completely clears the transposition table.
// Typically called at the start of a new game or when 
// requested by the GUI to reset engine state.
void irida_TTClear(void);

// @function tt_inc_generation
// @desc Call at start of each search_root.
void irida_TTIncGeneration(void);

// @function tt_probe
// @param key Position key.
// @param depth Stored depth.
// @param alpha Alpha bound.
// @param beta Beta bound.
// @param ply Current ply.
// @param outScore Output score (if hit).
// @param outMove Output best move (if hit).
// @returns bool True if probe hit and score/move are valid.
bool irida_TTProbe(uint64_t key,
              int depth,
              int alpha,
              int beta,
              int ply,
              int* outScore,
              Move* outMove);

// @function tt_probe_pv
// @desc For PV construction: return stored best move if key and generation match (ignore depth).
// @returns bool True if entry exists and *outMove was set.
bool irida_TTProbePV(uint64_t key, Move* outMove);

// @function tt_store
// @param key Position key.
// @param depth Depth of score.
// @param score Score to store.
// @param type TT_EXACT, TT_LOWERBOUND, or TT_UPPERBOUND.
// @param bestMove Best move at this node.
// @param ply Current ply (for generation).
void irida_TTStore(uint64_t key,
              int depth,
              int score,
              TTNodeType type,
              Move bestMove,
              int ply);

// @function tt_hashfull
// @desc Returns the "hashfull" value for UCI.
// Measures how many slots in the first 1000 entries are occupied 
// by the current search generation. Returns a value from 0 to 1000.
int irida_TTHashfull(void);

// @function tt_adjust_score
// @desc Converts a search score (relative to the root) into a 
// position-independent score for storage in the TT.
int irida_TTAdjustScore(int stored, int ply);

// @function tt_unadjust_score
// @desc Converts a stored TT score (position-independent) back into 
// a search score relative to the current search ply.
int irida_TTUnadjustScore(int stored, int ply);

#endif // TT_H
