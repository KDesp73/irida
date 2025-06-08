#ifndef ENGINE_ZOBRIST_H
#define ENGINE_ZOBRIST_H

#include "board.h"
#include <stdint.h>

#define CASTLING_OPTIONS 4

extern uint64_t zobrist_table[PIECE_TYPES][BOARD_SIZE][BOARD_SIZE] __attribute__((unused));
extern uint64_t zobrist_castling[CASTLING_OPTIONS]                 __attribute__((unused));
extern uint64_t zobrist_en_passant[BOARD_SIZE]                     __attribute__((unused));
extern uint64_t zobrist_black_to_move                              __attribute__((unused));

// NOTE: Not used since we are using polyglot's standard 781 "random" values
void InitZobrist();

uint64_t CalculateZobristHash(const Board* board);
static inline uint64_t CalculateZobristHashFen(const char* fen)
{
    Board b;
    BoardInitFen(&b, fen);
    uint64_t hash = CalculateZobristHash(&b);
    BoardFree(&b);
    return hash;
}

#endif // ENGINE_ZOBRIST_H
