#ifndef NNUE_H
#define NNUE_H

#include "castro.h"
#include <stdbool.h>

// @module nnue
// @desc NNUE network load/query; engine falls back to PeSTO if not loaded.

// @function NNUELoad
// @desc Load NNUE network from file. If no NNUE backend is linked, returns false and eval falls back to PeSTO.
// @param path Path to .nnue file.
// @returns bool True on success.
bool irida_NNUELoad(const char* path);

// @function NNUEAvailable 
// @desc Whether an NNUE net is loaded and ready.
// @returns bool True if NNUE is loaded.
bool irida_NNUEAvailable(void);

#define NNUE_DEFAULT_PATH "~/.irida/nn/nn-04cf2b4ed1da.nnue"

/* Optional incremental NNUE (search must call Begin/End and make/unmake hooks). */
void irida_NNUEBuildPsq(const Board* board, int* pieces, int* squares);
int irida_NNUEEvalWithPsq(const Board* board);
int irida_NNUEEvalIncrementalForBoard(const Board* board);
void irida_NNUEAccSearchBegin(Board* board);
void irida_NNUEAccSearchEnd(void);
void irida_NNUEAccBeforeChild(const Board* parent, Move move);
void irida_NNUEAccCommitChild(void);
void irida_NNUEAccPop(void);
void irida_NNUENullMoveEnter(void);
void irida_NNUENullMoveExit(void);
bool irida_NNUEAccShouldUseIncremental(void);
bool irida_NNUEAccSessionActive(void);
bool irida_NNUEAccInNullMove(void);

#endif
