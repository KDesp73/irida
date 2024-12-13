#include "board.h"
#include <chess/board.h>
#include <chess/ui.h>
#include <stdio.h>

int main(){
    Board board;
    BoardInitFen(&board, NULL);
    TuiBoardPrint(&board, MINIMAL_CONFIG, 64);

    board_free(&board);
    return 0;
}
