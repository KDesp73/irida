int test_eval_log(const char* fen);

#define TAG "eval_log"
#include "IncludeOnly/test.h"
#include "castro.h"
#include "eval.h"
#include <stdio.h>

int test_eval_log(const char* fen)
{
    Board board;
    castro_BoardInitFen(&board, fen);
    fprintf(stderr, "[eval] FEN: %s\n", fen);
    EvalBreakdown b = {0};
    eval_breakdown(&board, &b);
    log_breakdown(b);
    castro_BoardFree(&board);
    SUCC("logged breakdown");
    return 1;
}
