#include "tests.h"
#include "movegen.h"
#include "test-perft.h"
#include "perft.h"

int test_perft_captures_3(int depth, u64 expected)
{
    PERFT_TEST("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", depth, MOVE_CAPTURES, expected);
}


