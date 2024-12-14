#include "bitboard.h"
#include "board.h"
#include "square.h"
#include <chess/board.h>
#include <chess/ui.h>
#include <stdio.h>

int main(){
    Board board;
    BoardInitFen(&board, "rnbq3r/p1ppkP1p/1p2p2b/8/2P5/2N1PN2/PP1P2pP/R1BQKB1R w KQ - 0 1");
    BitboardPrint(WhitePawnPromotions(board.bitboards[INDEX_WHITE_PAWN], GetEmpty(&board)));
    BitboardPrint(BlackPawnPromotions(board.bitboards[INDEX_BLACK_PAWN], GetEmpty(&board)));
    BitboardPrint(WhitePawnPromotionCaptures(board.bitboards[INDEX_WHITE_PAWN], GetBlack(&board)));
    BitboardPrint(BlackPawnPromotionCaptures(board.bitboards[INDEX_BLACK_PAWN], GetWhite(&board)));

    board_free(&board);
    return 0;
}
