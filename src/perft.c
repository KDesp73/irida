#include "perft.h"
#include "board.h"
#include "movegen.h"
#include <stdio.h>

#define PRINTALL false

u64 Perft(Board* board, int depth, bool root, const char* parent)
{
    uint64_t cnt = 0, nodes = 0;
    bool leaf = (depth == 2);

    Moves moves = GenerateLegalMoves(board);
    for (int i = 0; i < moves.count; i++) {
        Move move = moves.list[i];
        char moveStr[16];
        MoveToString(move, moveStr);

        if (root && depth <= 1) {
            cnt = 1;
            nodes++;
        } else {
        if (!MakeMove(board, move)) continue;
            cnt = leaf ? GenerateLegalMoves(board).count : Perft(board, depth - 1, PRINTALL, moveStr);
            nodes += cnt;

            UnmakeMove(board);
        }

        if (root) {
            if(parent){
                printf("%s -> ", parent);
            }
            printf("%s: %lu\n", moveStr, cnt);
        }
    }

    return nodes;
}
