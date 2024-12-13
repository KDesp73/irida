#define TEST_IMPLEMENTATION
#include <io/test.h>
#include "tests.h"

int main() {
    LOAD_TEST("test_piece_at");

    START_TESTS
        RUN_TEST(test_piece_at)
    END_TESTS
}
