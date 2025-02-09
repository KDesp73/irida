#include "perft.h"
#include "board.h"
#include "move.h"
#include "movegen.h"
#include <stdio.h>

u64 Perft(Board* board, int depth, MoveType type, bool root)
{
    uint64_t cnt = 0, nodes = 0;
    bool leaf = (depth == 2);

    Moves moves = GenerateMoves(board, type);
    for (int i = 0; i < moves.count; i++) {
        Move move = moves.list[i];

        if (root && depth <= 1) {
            cnt = 1;
            nodes++;
        } else {
        if (!MakeMove(board, move)) continue;
            cnt = leaf ? GenerateMoves(board, type).count : Perft(board, depth - 1, type, false);
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
