#define TEST_IMPLEMENTATION
#include <io/test.h>
#include "tests.h"

void load()
{
    LOAD_TEST("test_piece_at");
    LOAD_TEST("test_name_to_square");
    LOAD_TEST("test_square_to_name");
}

int main(int argc, char** argv)
{
    load();

    START_TESTS
        RUN_TEST(test_piece_at),
        RUN_TEST(test_name_to_square),
        RUN_TEST(test_square_to_name)
    END_TESTS
}
