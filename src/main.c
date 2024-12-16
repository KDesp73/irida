#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include "move.h"
#include "square.h"
#include <chess/board.h>
#include <chess/ui.h>
#include <chess/zobrist.h>
#include <stdio.h>

int main(){
    init_zobrist();
    InitializeMasks();

    Board board;
    BoardInitFen(&board, "rnb1kbnr/pp3ppp/2p5/3Pp3/7q/2N5/PPPP1PPP/R1BQKBNR w KQkq e6 0 1");
    
    Bitboard moves = GenerateBlackPawnMoves(&board);
    BoardPrintBitboard(&board, MINIMAL_CONFIG, moves);

    board_free(&board);
    return 0;
}
