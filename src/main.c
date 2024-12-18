#include "bitboard.h"
#include "board.h"
#include "history.h"
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
    BoardInitFen(&board,"rnbqkbnr/pp1pp1pp/8/4Pp2/1Pp5/8/P1PP1PPP/RNBQKBNR b KQkq b3 0 1");

    BoardPrint(&board, 64);
    // BoardPrintGrid(&board);

    Move move = MoveEncodeNames("c4", "b3", PROMOTION_NONE, FLAG_NORMAL);
    MakeMove(&board, move);

    BoardPrintMove(&board, move);
    BoardPrintGrid(&board);

    UnmakeMove(&board);

    BoardPrintMove(&board, move);
    BoardPrintGrid(&board);

    BoardFree(&board);

    return 0;
}
