#include "perft.h"
#include "board.h"
#include "movegen.h"
#include <stdio.h>

#define PRINTALL false

u64 Perft(Board* board, int depth, bool root)
{
    int n_moves, i;
    u64 nodes = 0;

    // Generate all legal moves and store them in move_list
    Moves moves = GenerateLegalMoves(board);
    n_moves = moves.count;

    // If at the last depth, return the number of legal moves
    if (depth == 1) 
        return (u64)n_moves;

    // Iterate through all generated moves
    for (i = 0; i < n_moves; i++) {
        Move move = moves.list[i];
        char moveStr[16];
        MoveToString(move, moveStr);

        // Attempt to make the move
        if (!MakeMove(board, move)) 
            continue; // Skip illegal moves

        // Recursively count nodes for the next depth
        nodes += Perft(board, depth - 1, false);

        // Undo the move
        UnmakeMove(board);
        if (root) {
            printf("%s: %llu\n", moveStr, nodes);
        }
    }

    return nodes;
}
