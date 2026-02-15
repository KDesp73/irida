#ifndef SEARCH_H
#define SEARCH_H

#include "castro.h"
#include "eval.h"

#define INF 1000000

typedef Move (*SearchFn)(Board* board, EvalFn eval, int depth);

Move alpha_beta_search(Board* board, EvalFn eval, int depth);
Move iterative_deepening(Board* board, EvalFn eval, int maxDepth);

int negamax(Board* board, int depth, int alpha, int beta, EvalFn eval);
int quiescence(Board* board, int alpha, int beta, EvalFn eval);
int negamax_quiscence(Board* board, int depth, int alpha, int beta, EvalFn eval);

#endif // SEARCH_H
