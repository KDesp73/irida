#include "bitboard.h"

Bitboard WhitePawnAttacks(Bitboard pawns)
{
    uint64_t left_attacks = (pawns & ~FILE_A) << 7;
    uint64_t right_attacks = (pawns & ~FILE_H) << 9;

    return left_attacks | right_attacks;
}

Bitboard BlackPawnAttacks(Bitboard pawns)
{
    uint64_t left_attacks = (pawns & ~FILE_H) >> 7;
    uint64_t right_attacks = (pawns & ~FILE_A) >> 9;

    return left_attacks | right_attacks;
}

Bitboard KnightAttacks(Bitboard knights)
{
    return ((knights << 6) & ~FILE_H & ~FILE_G)
         | ((knights << 10) & ~FILE_A & ~FILE_B)
         | ((knights >> 6) & ~FILE_A & ~FILE_B)
         | ((knights >> 10) & ~FILE_H & ~FILE_G)
         | ((knights << 15)  & ~FILE_H)
         | ((knights << 17) & ~FILE_A)
         | ((knights >> 15) & ~FILE_A)
         | ((knights >> 17) & ~FILE_H); 
}

Bitboard KingAttacks(Bitboard king)
{
    return ((king << 1) & ~FILE_A)
         | ((king << 7) & ~FILE_H)
         | ((king << 8))
         | ((king << 9) & ~FILE_A)
         | ((king >> 1) & ~FILE_H)
         | ((king >> 7) & ~FILE_A)
         | ((king >> 8))
         | ((king >> 9) & ~FILE_H);
}

