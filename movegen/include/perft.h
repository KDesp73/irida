#ifndef ENGINE_PERFT_H
#define ENGINE_PERFT_H


#include "board.h"
#include "move.h"
#include "movegen.h"
#include <stdio.h>

typedef unsigned long long u64;

// https://www.chessprogramming.org/Perft
static inline u64 Perft(Board* board, int depth, bool root)
{
    uint64_t cnt = 0, nodes = 0;
    bool leaf = (depth == 2);

    Moves moves = GenerateMoves(board, MOVE_LEGAL);
    for (int i = 0; i < moves.count; i++) {
        Move move = moves.list[i];

        if (root && depth <= 1) {
            cnt = 1;
            nodes++;
        } else {
            if (!MakeMove(board, move)) continue;
            cnt = leaf ? GenerateMoves(board, MOVE_LEGAL).count : Perft(board, depth - 1, false);
            nodes += cnt;

            UnmakeMove(board);
        }

        if (root) {
            char moveStr[16];
            MoveToString(move, moveStr);
            printf("%s: %lu\n", moveStr, cnt);
        }
    }

    return nodes;
}

#endif // ENGINE_PERFT_H
