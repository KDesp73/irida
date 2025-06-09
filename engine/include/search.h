#ifndef ENGINE_SEARCH_H
#define ENGINE_SEARCH_H

#include "board.h"
#include "move.h"

/*
 * =Search Plan=
 *
 * 1. Negamax
 * 2. Alpha-Beta Pruning
 * 3. Iterative Deepening
 */


int Minimax(Board* board, int depth, bool isMaximizing);
Move FindBest(Board* board, int depth, int* score);

#endif //  ENGINE_SEARCH_H
