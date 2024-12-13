#ifndef ENGINE_ZOBRIST_H
#define ENGINE_ZOBRIST_H

#include "board.h"
#include <stdint.h>

uint64_t CalculateZobristHash(const Board* board);

#endif // ENGINE_ZOBRIST_H
