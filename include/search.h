#ifndef SEARCH_H
#define SEARCH_H

#include "castro.h"
#include "eval.h"

typedef Move (*SearchFn)(Board* board, EvalFn eval, int depth);

Move alpha_beta_search(Board* board, EvalFn eval, int depth);

#endif // SEARCH_H
