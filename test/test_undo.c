#include <io/test.h>
#include "board.h"
#include "tests.h"
#include "move.h"

#define DEBUG
#include <io/logging.h>

int test_undo(const char* fen, Move move)
{
    Board board;
    BoardInitFen(&board, fen);
    Board originalBoard = board;
    
    if (!MakeMove(&board, move)) {
        FAIL("MakeMove failed for move: ");
        MovePrint(move);
        return false;
    }

    UnmakeMove(&board);

    for(size_t i = 0; i < PIECE_TYPES; i++){
        if(originalBoard.bitboards[i] != board.bitboards[i]) {
            FAILF(fen, "Mismatch in bitboard index %zu", i);
            return 0;
        }
    }

    if(board.enpassant_square != originalBoard.enpassant_square){
        FAILF(fen, "Mismatch in enpassant square. Expected %d. Found %d", originalBoard.enpassant_square, board.enpassant_square);
        return 0;
    }

    if(board.castling_rights != originalBoard.castling_rights){
        FAILF(fen, "Mismatch in castling rights");
        return 0;
    }

    if(board.turn != originalBoard.turn) {
        FAILF(fen, "Mismatch in turn");
        return 0;
    }

    if(board.fullmove != originalBoard.fullmove) {
        FAILF(fen, "Mismatch in fullmove");
        return 0;
    }

    if(board.halfmove != originalBoard.halfmove) {
        FAILF(fen, "Mismatch in halfmove");
        return 0;
    }

    return 1;
}
