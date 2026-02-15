#ifndef EVAL_H
#define EVAL_H

#include "castro.h"

typedef int (*EvalFn)(Board*) ;

int material_eval(Board* board);
int pesto_eval(Board* board);

#endif // EVAL_H
