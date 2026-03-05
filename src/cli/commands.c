#include "IncludeOnly/logging.h"
#include "cli.h"
#include "castro.h"
#include "core.h"
#include "eval.h"
#include "nnue.h"
#include "search.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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

EvalFn eval_dispatcher(char* eval) {
    if(!eval) return engine.eval;
    if(!strcmp(eval, "material")) return material_eval;
    if(!strcmp(eval, "pesto")) return pesto_eval;
    if(!strcmp(eval, "nnue")) return nnue_eval;
    return engine.eval;
}

SearchFn search_dispatcher(char* search) {
    if(!search) return engine.search;
    // TODO: add variants
    return engine.search;
}

bool eval_handler(Context context)
{
    engine.eval = eval_dispatcher(context.eval);
    engine.search = search_dispatcher(context.search);

    castro_BoardInitFen(&engine.board, context.fen);
    int eval = engine.eval(&engine.board);

    castro_BoardFree(&engine.board);

    printf("%.1f\n", (double) eval / 100);
    return true;
}

bool search_handler(Context context)
{
    engine.eval = eval_dispatcher(context.eval);
    engine.search = search_dispatcher(context.search);

    if(!context.depth) {
        ERRO("Provide a depth");
        return false;
    }

    castro_BoardInitFen(&engine.board, context.fen);

    char bestmove[16];
    Move move = engine.search(&engine.board, engine.eval, engine.order, &g_searchConfig);

    castro_BoardFree(&engine.board);

    castro_MoveToString(move, bestmove);
    printf("%s\n", bestmove);

    return true;
}
