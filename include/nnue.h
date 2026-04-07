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

#endif
