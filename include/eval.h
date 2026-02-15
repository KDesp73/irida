#ifndef EVAL_H
#define EVAL_H

#include "castro.h"

typedef int (*EvalFn)(Board*) ;

int simple_eval_fn(Board* board);

#endif // EVAL_H
