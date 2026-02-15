#include "IncludeOnly/logging.h"
#include "cli.h"
#include "castro.h"
#include "core.h"
#include <stdbool.h>
#include <stdio.h>

bool perft_handler(Context context)
{
    char* fen = context.fen;
    size_t depth = context.depth;

    if(!depth) {
        ERRO("Provide a depth");
        return false;
    }

    Board board = {0};
    castro_BoardInitFen(&board, fen);

    u64 moves = castro_Perft(&board, depth, false);

    printf("%llu\n", moves);

    castro_BoardFree(&board);

    return true;
}

bool eval_handler(Context context)
{
    castro_BoardInitFen(&engine.board, context.fen);
    int eval = engine.eval(&engine.board);

    castro_BoardFree(&engine.board);

    printf("%.1f\n", (double) eval / 100);
    return true;
}

bool search_handler(Context context)
{
    castro_BoardInitFen(&engine.board, context.fen);

    char bestmove[16];
    Move move = engine.search(&engine.board, engine.eval, 4);

    castro_BoardFree(&engine.board);

    castro_MoveToString(move, bestmove);
    printf("%s\n", bestmove);

    return true;
}
