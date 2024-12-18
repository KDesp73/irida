#include "perft.h"
#include "tests.h"
#include <io/test.h>

int test_perft_2(int depth, u64 expected)
{
    Board board;
    BoardInitFen(&board, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1");

    u64 count = Perft(&board, depth);
    BoardFree(&board);

    if(count != expected){
        FAIL("For depth %d. Expected %llu. Found %llu", depth, expected, count);
        return 0;
    }

    SUCC("For depth %d", depth);
    return 1;
}

