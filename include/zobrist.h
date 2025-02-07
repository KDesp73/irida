#ifndef ENGINE_ZOBRIST_H
#define ENGINE_ZOBRIST_H

#include "board.h"
#include <stdint.h>

#define CASTLING_OPTIONS 4

static uint64_t zobrist_table[PIECE_TYPES][BOARD_SIZE][BOARD_SIZE] __attribute__((unused));
static uint64_t zobrist_castling[CASTLING_OPTIONS]                 __attribute__((unused));
static uint64_t zobrist_en_passant[BOARD_SIZE]                     __attribute__((unused));
static uint64_t zobrist_black_to_move                              __attribute__((unused));

void InitZobrist();

uint64_t CalculateZobristHash(const Board* board);

#endif // ENGINE_ZOBRIST_H
