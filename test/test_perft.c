#include "perft.h"
#include "tests.h"
#include <io/test.h>

int test_perft(int depth, u64 expected)
{
    Board board;
    BoardInitFen(&board, NULL);
    u64 count = Perft(&board, depth);
    board_free(&board);

    if(count != expected){
        FAIL("For depth %d. Expected %llu. Found %llu", depth, expected, count);
        return 0;
    }

    SUCC("For depth %d", depth);
    return 1;
}
