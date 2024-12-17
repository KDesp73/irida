#include "bitboard.h"
#include "board.h"
#include "piece.h"
#include "square.h"
#include "tests.h"
#include <stdarg.h>
#include <io/test.h>

int test_bishop_moves(const char* fen, const char* square, const char* first, ...)
{
    Board board;
    BoardInitFen(&board, fen);

    Square from = SquareFromName(square);

    Bitboard moves = 0ULL;
    if(first != NULL){
        va_list args;
        va_start(args, first);
        on(&moves, SquareFromName(first));
        const char* next = va_arg(args, const char*);
        while(next != NULL){
            on(&moves, SquareFromName(next));
            next = va_arg(args, const char*);
        }
        va_end(args);
    }

    Color color = PieceAt(&board, from).color;
    Bitboard found = GenerateBishopMoves(&board, 1ULL << from, color);
    if(found != moves){
        FAILF(fen, "For square %s", square);
        printf("Expected: ");Uint64Print(moves);
        printf("Found:    ");Uint64Print(found);
        return false;
    }

    SUCC("Passed");
    return true;
}
