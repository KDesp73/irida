#include "move.h"

void MovesAppend(Moves* moves, Move move)
{
    if(moves->count >= MOVES_CAPACITY) return;

    moves->list[moves->count++] = move;
}

void MovesAppendList(Moves* dest, Moves src)
{
    for (size_t i = 0; i < src.count && dest->count < MOVES_CAPACITY; i++) {
        dest->list[dest->count++] = src.list[i];
    }
}
