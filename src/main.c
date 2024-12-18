#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include "perft.h"
#include <chess/board.h>
#include <chess/ui.h>
#include <chess/zobrist.h>
#include <io/logging.h>

void perft(int argc, char** argv)
{
    if(argc != 2) {
        ERRO("Specify the depth");
        return;
    }

    Board board;
    BoardInitFen(&board, NULL);
    u64 count = Perft(&board, atoi(argv[1]));
    INFO("count: %llu", count);
    board_free(&board);
}

int main(int argc, char** argv){
    init_zobrist();
    InitializeMasks();

    perft(argc, argv);

    return 0;
}
