#include <string.h>
#define TEST_IMPLEMENTATION
#include <io/test.h>
#include "tests.h"

void load()
{
    LOAD_TEST("test_piece_at");
    LOAD_TEST("test_name_to_square");
    LOAD_TEST("test_perft");
    LOAD_TEST("test_square_to_name");

    LOAD_TEST("test_pawn_moves");
    LOAD_TEST("test_knight_moves");
    LOAD_TEST("test_bishop_moves");
    LOAD_TEST("test_rook_moves");
    LOAD_TEST("test_queen_moves");
    LOAD_TEST("test_king_moves");

    LOAD_TEST("test_pawn_pseudo");
    LOAD_TEST("test_knight_pseudo");
    LOAD_TEST("test_bishop_pseudo");
    LOAD_TEST("test_rook_pseudo");
    LOAD_TEST("test_queen_pseudo");
    LOAD_TEST("test_king_pseudo");
}

int main(int argc, char** argv)
{
    if(argc == 2 && !strcmp(argv[1], "load")){
        load();
        return 0;
    }

    START_TESTS
        // RUN_TEST(test_piece_at),
        // RUN_TEST(test_name_to_square),
        // RUN_TEST(test_square_to_name),
        // RUN_TEST(test_perft),

        // RUN_TEST(test_pawn_moves),
        // RUN_TEST(test_knight_moves),
        // RUN_TEST(test_bishop_moves),
        // RUN_TEST(test_rook_moves),
        // RUN_TEST(test_queen_moves),
        RUN_TEST(test_king_moves),
        //
        // RUN_TEST(test_pawn_pseudo),
        // RUN_TEST(test_knight_pseudo),
        // RUN_TEST(test_bishop_pseudo),
        // RUN_TEST(test_rook_pseudo),
        // RUN_TEST(test_queen_pseudo),
        RUN_TEST(test_king_pseudo),

        1
    END_TESTS
}
