#ifndef NNUE_H
#define NNUE_H

#include "castro.h"
#include <stdbool.h>

// @module nnue
// @desc NNUE network load/query; engine falls back to PeSTO if not loaded.

// @function nnue_load
// @desc Load NNUE network from file. If no NNUE backend is linked, returns false and eval falls back to PeSTO.
// @param path Path to .nnue file.
// @returns bool True on success.
bool nnue_load(const char* path);

// @function nnue_available
// @desc Whether an NNUE net is loaded and ready.
// @returns bool True if NNUE is loaded.
bool nnue_available(void);

#endif
