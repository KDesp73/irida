#include "syzygy.h"
#include "IncludeOnly/kv.h"
#include "IncludeOnly/logging.h"
#include "castro.h"

int main(int argc, char** argv)
{
    kv_parse(argc, argv);
    const char* fen = kv_get("fen", STARTING_FEN);
    const char* path = kv_get("path", NULL);

    if(!path) {
        ERRO("Please provide a tb path");
        return 1;
    }

    Board board = {0};
    castro_BoardInitFen(&board, fen);


    if(!syzygy_init(path)) {
        ERRO("Could not initialize syzygy");
        return 1;
    }
    if(!syzygy_available()) {
        ERRO("Syzygy is not available");
        return 2;
    }


    Move best;
    if(!syzygy_probe_root(&board, true, &best)){
        ERRO("Could not probe best move");
        return 3;
    }

    char buf[12];
    castro_MoveToString(best, buf);

    printf("%s\n", buf);

    castro_BoardFree(&board);
    return 0;
}
