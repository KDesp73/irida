#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include "square.h"
#include <chess/board.h>
#include <chess/ui.h>
#include <chess/zobrist.h>
#include <stdio.h>

int main(){
    init_zobrist();
    InitializeMasks();

    Board board;
    BoardInitFen(&board, "1k6/8/1p3R2/4r3/8/2Q1B3/8/1Kn3P1 w - - 0 1");
    
    BoardPrintBitboard(&board, MINIMAL_CONFIG, WhitePawnPushes(board.bitboards[INDEX_WHITE_PAWN], GetEmpty(&board)));

    // for(size_t i = 0; i < 64; i++){
    //     BitboardPrint(KnightAttacks(1ULL << i));
    //     getchar();
    // }
    //
    board_free(&board);
    return 0;
}
