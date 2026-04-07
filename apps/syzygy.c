/*
 * syzygy.c
 *
 * Get syzygy result for a specific endgame position
 * NOTE: run ./scripts/download/tablebase first
 *
 * Usage: ./syzygy [path=<PATH-TO-TB>] fen="ENDGAME-FEN"
 */

#include "syzygy.h"
#include "IncludeOnly/kv.h"
#include "IncludeOnly/logging.h"
#include "castro.h"

int main(int argc, char** argv)
{
    kv_parse(argc, argv);
    const char* fen = kv_get("fen", STARTING_FEN);
    const char* path = kv_get("path", SYZYGY_DEFAULT_PATH);

    if(!path) {
        ERRO("Please provide a tb path");
        return 1;
    }

    if(!irida_SyzygyInit(path)) {
        ERRO("Could not initialize syzygy");
        return 1;
    }

    if(!irida_SyzygyAvailable()) {
        ERRO("Syzygy is not available");
        return 2;
    }

    Board board = {0};
    castro_BoardInitFen(&board, fen);
    Move best;

    if(!irida_SyzygyProbeRoot(&board, true, &best)){
        ERRO("Could not probe best move");
        castro_BoardFree(&board);
        return 3;
    }

    char buf[12];
    castro_MoveToString(best, buf);

    printf("%s\n", buf);

    castro_BoardFree(&board);
    return 0;
}
