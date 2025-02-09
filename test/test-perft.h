#ifndef TEST_PERFT_MACRO_H
#define TEST_PERFT_MACRO_H

#include "extern/test.h"
#define PERFT_TEST(fen, depth, type, expected) \
    Board board; \
    BoardInitFen(&board, fen); \
    u64 count = Perft(&board, depth, type, true); \
    BoardFree(&board); \
\
    if(count != expected){ \
        FAIL("For depth %d. Expected %llu. Found %llu", depth, expected, count); \
        return 0; \
    } \
 \
    SUCC("For depth %d", depth); \
    return 1

#endif // TEST_PERFT_MACRO_H
