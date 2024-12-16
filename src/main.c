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
    
    Square from = SquareFromName("d2");
    Square to = SquareFromName("d3");
    printf("%d\n", from);
    SQUARE_PRINT(from);
    SQUARE_PRINT(to);
    DoMove(&board.bitboards[INDEX_WHITE_PAWN], MoveEncode(from, to, PROMOTION_NONE, FLAG_NORMAL));
    BitboardPrint(board.bitboards[INDEX_WHITE_PAWN]);

    return 0;

    Bitboard moves = GenerateWhitePawnMoves(board.bitboards[INDEX_WHITE_PAWN], board.bitboards[INDEX_WHITE_KING], GetEmpty(&board), GetEnemy(&board), board.enpassant_square);
    BoardPrintBitboard(&board, MINIMAL_CONFIG, moves);

    board_free(&board);
    return 0;
}
