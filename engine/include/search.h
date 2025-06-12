#ifndef ENGINE_SEARCH_H
#define ENGINE_SEARCH_H

#include "board.h"
#include "move.h"

// See: https://www.chessprogramming.org/Negamax

#define MAX_DEPTH 200

int Negamax(Board* board, int depth, int alpha, int beta);
Move FindBest(Board* board, int depth, int* score);
Move FindBestIterative(Board* board, int maxDepth, int *outScore);

// TODO:
// 1. Principal Variation Search
// 2. Trasposition Tables
// 3. Late Move Reductions
// 4. Razoring
// 5. Quiescence Search
// 6. Aspiration Window

typedef struct {
    uint64_t key;        // Zobrist hash of the position
    int depth;           // Depth at which the evaluation was done
    int score;           // Evaluation score (in centipawns)
    Move bestMove;       // Best move found at this node
    enum { EXACT, LOWERBOUND, UPPERBOUND } flag;
} TTEntry;


#endif //  ENGINE_SEARCH_H
