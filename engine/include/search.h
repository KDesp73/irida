#ifndef ENGINE_SEARCH_H
#define ENGINE_SEARCH_H

#include "board.h"
#include "move.h"

// See: https://www.chessprogramming.org/Negamax
int Negamax(Board* board, int depth, int alpha, int beta);
Move FindBest(Board* board, int depth, int* score);
Move FindBestIterative(Board* board, int maxDepth, int *outScore);

// TODO:
// 1. Principal Variation Search
// 2. Trasposition Tables
// 3. Late Move Reductions
// 4. Null Move Pruning
// 5. Razoring
// 6. Quiescence Search
// 7. Aspiration Window


#endif //  ENGINE_SEARCH_H
