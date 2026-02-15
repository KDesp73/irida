#include "castro.h"
#include "eval.h"
#include "moveordering.h"
#include "search.h"

Move iterative_deepening(Board* board, EvalFn eval, int maxDepth) {
    Move bestMove = {0};
    for (int depth = 1; depth <= maxDepth; depth++) {
        Moves moves = castro_GenerateMoves(board, MOVE_LEGAL);
        int bestScore = -INF;

        order_moves(board, moves.list, moves.count, depth);

        for (size_t i = 0; i < moves.count; i++) {
            Move move = moves.list[i];
            if (!castro_MakeMove(board, move)) continue;

            int score = -negamax_quiscence(board, depth - 1, -INF, INF, eval);
            castro_UnmakeMove(board);

            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
    }

    return bestMove;
}
