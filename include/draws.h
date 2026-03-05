#ifndef DRAWS_H
#define DRAWS_H

#include "castro.h"
#include <stdbool.h>

/* Returns true if the position is a draw by 50-move rule, threefold repetition, or insufficient material. */
bool is_draw(Board* board);

#endif
