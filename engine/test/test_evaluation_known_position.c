#include "IncludeOnly/test.h"
#include "board.h"
#include "evaluation.h"

int test_evaluation_known_position(char* position, int eval)
{
    Board board = {0};
    BoardInitFen(&board, position);

    Eval e = Evaluation(&board);
    BoardFree(&board);

    if(e.total != eval) {
        FAIL("Fail at %s. Expected: %d. Found: %d", position, eval, e.total);
        return 0;
    }

    SUCC("Passed");
    return 1;
}
