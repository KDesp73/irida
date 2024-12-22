#include "move.h"
#include <string.h>

void MovesAppend(Moves* moves, Move move)
{
    if(moves->count >= MOVES_CAPACITY) return;

    moves->list[moves->count++] = move;
}

void MovesAppendList(Moves* dest, Moves src)
{
    if (!dest) return;

    for (size_t i = 0; i < src.count && dest->count < MOVES_CAPACITY; i++) {
        dest->list[dest->count++] = src.list[i];
    }
}

Moves MovesCombine(Moves m1, Moves m2)
{
    Moves result = {0};

    memcpy(result.list, m1.list, m1.count * sizeof(Move));
    result.count = m1.count;

    for (size_t i = 0; i < m2.count && result.count < MOVES_CAPACITY; i++) {
        result.list[result.count++] = m2.list[i];
    }

    return result;
}
