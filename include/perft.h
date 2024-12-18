#ifndef ENGINE_PERFT_H
#define ENGINE_PERFT_H

#include "move.h"

typedef unsigned long long u64;

// https://www.chessprogramming.org/Perft
u64 Perft(Board* board, int depth);
u64 PerftLegal(Board* board, int depth);

#endif // ENGINE_PERFT_H
