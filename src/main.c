#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include <chess/board.h>
#include <chess/ui.h>
#include <chess/zobrist.h>

int main(){
    init_zobrist();
    InitializeMasks();

    Board board;
    BoardInitFen(&board, "r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/5N2/PPPP1PPP/RNBQK2R w KQkq - 0 1");
    
    Bitboard moves = GenerateWhiteKingMoves(&board);
    BoardPrintBitboard(&board, MINIMAL_CONFIG, moves);

    board_free(&board);
    return 0;
}
