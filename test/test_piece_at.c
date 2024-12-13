#include "board.h"
#include "piece.h"
#include "square.h"
#include <io/test.h>

int test_piece_at(const char* fen, Square square, char expected)
{
    Board board;
    BoardInitFen(&board, fen);

    char piece = PieceAt(&board, square).type;
    if(piece != expected){
        FAIL(fen, "Expected: %c. Found: %c", expected, piece);
        return 0;
    }
    SUCC("Passed");
    return 1;
}
