int test_eval(const char* fen, int score_min, int score_max);

#define TAG "eval"
#include "IncludeOnly/test.h"
#include "castro.h"
#include "eval.h"
#include <string.h>

int test_eval(const char* fen, int score_min, int score_max)
{
    Board board;
    castro_BoardInitFen(&board, fen);
    int score = evaluation(&board);
    castro_BoardFree(&board);
    if (score < score_min || score > score_max) {
        FAIL("pesto_eval %s: got %d, expected [%d, %d]", fen ? "fen" : "startpos", score, score_min, score_max);
        return 0;
    }
    SUCC("pesto_eval in [%d, %d]", score_min, score_max);
    return 1;
}
