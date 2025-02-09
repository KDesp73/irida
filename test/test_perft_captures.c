#include "tests.h"
#include "movegen.h"
#include "test-perft.h"
#include "perft.h"

int test_perft_captures(int depth, u64 expected)
{
    PERFT_TEST(NULL, depth, MOVE_CAPTURES, expected);
}
