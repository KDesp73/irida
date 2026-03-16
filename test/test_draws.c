int test_draws(const char* fen, int expected);

#define TAG "draws"
#include "IncludeOnly/test.h"
#include "castro_additions.h"
#include "castro.h"
#include <string.h>

int test_draws(const char* fen, int expected)
{
    Board board;
    castro_BoardInitFen(&board, fen);
    bool result = castro_IsDraw(&board);
    castro_BoardFree(&board);
    int got = result ? 1 : 0;
    if (got != expected) {
        FAIL("is_draw expected %d got %d", expected, got);
        return 0;
    }
    SUCC("is_draw -> %d", expected);
    return 1;
}
