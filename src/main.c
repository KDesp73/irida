#include "board.h"
#include "piece.h"
#include <chess/board.h>
#include <chess/ui.h>
#include <stdio.h>

int main(){
    Board board;
    BoardInitFen(&board, NULL);
    TuiBoardPrint(&board, MINIMAL_CONFIG, 0, 7, 52, 63, 64);

    Piece piece = PieceAt(&board, 60);
    PIECE_PRINT(piece);

    board_free(&board);
    return 0;
}
