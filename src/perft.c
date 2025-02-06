#include "perft.h"
#include "board.h"
#include "movegen.h"
#include <stdio.h>

u64 Perft(Board* board, int depth, bool root)
{
    int n_moves, i;
    u64 nodes = 0;

    Moves moves = GenerateLegalMoves(board);
    n_moves = moves.count;

    if (depth == 1) 
        return (u64)n_moves;

    for (i = 0; i < n_moves; i++) {
        Move move = moves.list[i];
        char moveStr[16];
        MoveToString(move, moveStr);

        if (!MakeMove(board, move)) 
            continue;

        nodes += Perft(board, depth - 1, false);

        UnmakeMove(board);
        if (root) {
            printf("%s: %llu\n", moveStr, nodes);
        }
    }

    return nodes;
}
