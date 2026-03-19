/*
 * nnue.c
 *
 * This app runs nnue files and provides the resulting evaluation
 * Useful for validating custom nnue models
 *
 * Usage: ./nnue path=<NNUE-PATH> fen=<FEN>
 */

#include "nnue.h"
#include "IncludeOnly/kv.h"
#include "IncludeOnly/logging.h"
#include "castro.h"
#include "eval.h"


int main(int argc, char** argv)
{
    kv_parse(argc, argv);
    const char* path = kv_get("path", NULL);
    const char* fen = kv_get("fen", STARTING_FEN);

    if(!path) {
        ERRO("Please provide a valid path to a *.nnue file");
        return 1;
    }

    if(!nnue_load(path)) {
        ERRO("Could not load nnue %s", path);
        return 1;
    }

    Board board = {0};
    castro_BoardInitFen(&board, fen);

    int eval = nnue_eval(&board);
    printf("%.2f\n", (float) eval / 100);

    castro_BoardFree(&board);
    
    return 0;
}
