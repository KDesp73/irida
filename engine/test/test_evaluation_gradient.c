#include "IncludeOnly/test.h"
#include "board.h"
#include "evaluation.h"
int test_evaluation_gradient(char* worse, char* better) 
{
    Board worse_board = {0};
    BoardInitFen(&worse_board, worse);
    Eval worse_eval = Evaluation(&worse_board);
    BoardFree(&worse_board);

    Board better_board = {0};
    BoardInitFen(&better_board, better);
    Eval better_eval = Evaluation(&better_board);
    BoardFree(&better_board);

    if(worse_eval.total >= better_eval.total) {
        FAIL("Failed gradient at %s. %d >= %d", worse, worse_eval.total, better_eval.total);
        EvalPrint(worse_eval);
        EvalPrint(better_eval);
        return 0;
    }

    SUCC("Passed");
    return 1;
}
