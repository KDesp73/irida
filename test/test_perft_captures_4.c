#include "tests.h"
#include "movegen.h"
#include "test-perft.h"
#include "perft.h"

int test_perft_captures_4(int depth, u64 expected)
{
    PERFT_TEST("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", depth, MOVE_CAPTURES, expected);
}


