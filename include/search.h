#ifndef ENGINE_SEARCH_H
#define ENGINE_SEARCH_H

#include "board.h"
#include "move.h"

int Minimax(Board* board, int depth, bool isMaximizing);
Move FindBest(Board* board, int depth, int* score);

#endif //  ENGINE_SEARCH_H
