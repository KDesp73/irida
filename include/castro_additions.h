#ifndef DRAWS_H
#define DRAWS_H

#include "castro.h"
#include <stdbool.h>

// @module castro_additions
// @desc Methods that should be added to castro.h

// @function HasNonPawnMaterial 
// @desc Returns true if the position is a draw by 50-move rule, threefold repetition, or insufficient material.
// @param board Board to check.
// @returns bool True if draw.
bool castro_IsDraw(Board* board);

bool castro_HasNonPawnMaterial(Board* board);

#endif
