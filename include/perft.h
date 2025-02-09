#ifndef ENGINE_PERFT_H
#define ENGINE_PERFT_H


#include "board.h"
#include "movegen.h"
typedef unsigned long long u64;

// https://www.chessprogramming.org/Perft
u64 Perft(Board* board, int depth, MoveType type, bool root);

#endif // ENGINE_PERFT_H
