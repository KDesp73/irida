#ifndef ENGINE_SEARCH_H
#define ENGINE_SEARCH_H

#include "board.h"
#include "move.h"

// See: https://www.chessprogramming.org/Negamax
int Negamax(Board* board, int depth, int alpha, int beta);
Move FindBest(Board* board, int depth, int* score);
Move FindBestIterative(Board* board, int maxDepth, int *outScore);


#endif //  ENGINE_SEARCH_H
