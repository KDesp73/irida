#include "perft.h"
#include "tests.h"
#include <io/test.h>

int test_perft_5(int depth, u64 expected)
{
    Board board;
    BoardInitFen(&board, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ");

    u64 count = Perft(&board, depth);
    BoardFree(&board);

    if(count != expected){
        FAIL("For depth %d. Expected %llu. Found %llu", depth, expected, count);
        return 0;
    }

    SUCC("For depth %d", depth);
    return 1;
}


