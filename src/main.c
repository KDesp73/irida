#include "bitboard.h"
#include "board.h"
#include "generator.h"
#include "masks.h"
#include "move.h"
#include "perft.h"
#include "zobrist.h"
#include <io/logging.h>
#include <stdio.h>

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
    BoardFree(&board);
}


int main(int argc, char** argv){
    InitZobrist();
    InitMasks();

    Board board;
    BoardInitFen(&board, "r3kbnr/ppNp1ppp/4b3/1B1Pp2q/4nP2/4P1P1/PPP4P/R1BQK1NR b kq - 0 1");

    BoardPrintBitboard(&board, GeneratePseudoLegalAttacks(&board, COLOR_WHITE));

    return 0;
}
