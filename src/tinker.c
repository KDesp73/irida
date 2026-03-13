#include "castro.h"
#include "eval.h"

int main(int argc, char** argv)
{
    const char* fen = (argc == 2) ? argv[1] : NULL;

    Board board = {0};
    castro_BoardInitFen(&board, fen);
    
    EvalBreakdown b = {0};
    int eval = pesto_eval_breakdown(&board, &b);

    pesto_log_breakdown(b);
    printf("Evaluation: %.2f\n", (float)eval / 100);

    castro_BoardFree(&board);
    return 0;
}
