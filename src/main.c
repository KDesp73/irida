#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include "move.h"
#include "perft.h"
#include "square.h"
#include "zobrist.h"
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
    BoardFree(&board);
}

int main(int argc, char** argv){
    InitZobrist();
    InitMasks();

    Board board;
    BoardInitFen(&board,"6r1/1k5P/8/8/8/3K4/5p2/6R1 b - - 0 1");

    BoardPrint(&board, 64);
    BoardPrintGrid(&board);

    MakeMove(&board, 
            MoveEncode(
                SquareFromName("f2"), SquareFromName("g1"), 
                PROMOTION_QUEEN, FLAG_PROMOTION)
            );

    BoardPrint(&board, 64);
    BoardPrintGrid(&board);

    BoardFree(&board);

    return 0;
}
