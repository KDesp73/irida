#ifndef DRAWS_H
#define DRAWS_H

#include "castro.h"
#include <stdbool.h>

// @module draws
// @desc Draw detection: 50-move, threefold, insufficient material.

// @function is_draw
// @desc Returns true if the position is a draw by 50-move rule, threefold repetition, or insufficient material.
// @param board Board to check.
// @returns bool True if draw.
bool is_draw(Board* board);

#endif
