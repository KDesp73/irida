#include "perft.h"
#include "board.h"
#include "generator.h"

u64 Perft(Board* board, int depth)
{
    int i;
    int n_moves;
    u64 nodes = 0;

    Moves moves = GeneratePseudoLegalMoves(board);
    n_moves = moves.count;

    if (depth == 1) 
        return (u64) moves.count;

    for (i = 0; i < n_moves; i++) {
        MakeMove(board, moves.list[i]);
        if (!IsInCheck(board)) {
            nodes += Perft(board, depth - 1);
        }
        UnmakeMove(board);
    }

    return nodes;
}

u64 PerftLegal(Board* board, int depth)
{
    int i;
    int n_moves;
    u64 nodes = 0;

    Moves moves = GenerateLegalMoves(board);
    n_moves = moves.count;

    if (depth == 1) 
        return (u64) moves.count;

    for (i = 0; i < n_moves; i++) {
        MakeMove(board, moves.list[i]);
        nodes += Perft(board, depth - 1);
        UnmakeMove(board);
    }

    return nodes;
}
