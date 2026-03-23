#include "castro.h"
#include "search.h"
#include "utils.h"
#include <stdlib.h>

Move random_move(Board* board, EvalFn eval, OrderFn order, SearchConfig* config)
{
    UNUSED(eval);
    UNUSED(order);
    UNUSED(config);

    Moves legal = castro_GenerateLegalMoves(board);

    if (legal.count == 0) {
        return NULL_MOVE;
    }

    return legal.list[rand() % legal.count];
}
