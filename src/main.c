#include "bitboard.h"
#include "board.h"
#include "generator.h"
#include "masks.h"
#include "move.h"
#include "notation.h"
#include "perft.h"
#include "square.h"
#include "uci.h"
#include "zobrist.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <io/logging.h>
#define ANSI_IMPLEMENTATION
#include <io/ansi.h>
#define MENU_IMPLEMENTATION
#include <io/menu.h>

void perft(int argc, char** argv)
{
    if(argc != 2) {
        ERRO("Specify the depth");
        return;
    }

    Board board;
    BoardInitFen(&board, NULL);
    u64 count = Perft(&board, atoi(argv[1]), true);
    INFO("count: %llu", count);
    BoardFree(&board);
}

int flush_input()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
    return 0;
}

int move(Board* board)
{
    char move_input[6]; // +1 for \0, +1 for safety
    printf("Enter move (e.g., e2e4): ");
    if (scanf("%5s", move_input) != 1) {
        flush_input(); // Clear input buffer
        printf("Invalid input. Try again.\n");
        return false;
    }
    flush_input(); // Clear any leftover input

    if (strlen(move_input) != 4 && strlen(move_input) != 5) {
        printf("Invalid move format. Use 4 or 5 characters (e.g., e2e4 or h7h8Q).\n");
        return false;
    }

    char from[3], to[3], promotion = '\0';
    strncpy(from, move_input, 2);
    from[2] = '\0';
    strncpy(to, move_input + 2, 2);
    to[2] = '\0';
    if (strlen(move_input) == 5) {
        promotion = move_input[4];
    }

    Square src = SquareFromName(from);
    Square dst = SquareFromName(to);
    uint8_t prom = CharToPromotion(promotion);
    Move move = MoveEncode(src, dst, prom, FLAG_NORMAL);

    if(!IsLegal(board, move)){
        printf("Illegal Move\n");
        return false;
    }
    
    if (!MakeMove(board, move)) {
        printf("Invalid move. Try again.\n");
        return false;
    }

    ansi_clear_screen();
    BoardPrintMove(board, move);
    return true;
}

void BoardInfoPrint(const Board* board)
{
    printf("Turn: %s\n", board->turn ? "White" : "Black");
    printf("Castling rights: %d\n", board->castling_rights);
    char name[3] = "-";
    if(board->enpassant_square != 64)
        SquareToName(name, board->enpassant_square);
    printf("Enpassant: %s\n", name);
}
#define BOARD_PRINT(board) \
    do { \
        BoardPrint(&board, 64); \
        BoardInfoPrint(&board); \
    } while(0)

int game(const char* fen)
{
    Board board;
    BoardInitFen(&board, fen);
    ansi_clear_screen();
    BOARD_PRINT(board);

    char fenExport[256];
    while(1){
        int option = menu("Options", 3, menu_arrow_print_option, "Move", "Undo", "Legal", "Pseudo", "Export Fen", "Exit", NULL);
        ansi_clear_screen();
        switch (option) {
        case 0:
            BOARD_PRINT(board);
            move(&board);
            break;
        case 1:
            UnmakeMove(&board);
            BOARD_PRINT(board);
            break;
        case 2:
            BoardPrintBitboard(&board, GenerateLegalMovesBitboard(&board));
            BoardInfoPrint(&board);
            break;
        case 3: 
            BoardPrintBitboard(&board, GeneratePseudoLegalMovesBitboard(&board));
            BoardInfoPrint(&board);
            break;
        case 4:
            FenExport(&board, fenExport);
            printf("%s\n", fenExport);
            break;
        case 5:
            menu_enable_input_buffering();
            exit(0);
        }
    }
    return 0;
}

#define forrange(index, from, to) \
    for(index = from; (from < to) ? i < to : i > to; (from < to) ? i++ : i--)

void legal()
{
    Board board;
    BoardInitFen(&board, "2kr2r1/ppp1bP1p/6P1/4q3/2p5/P2p4/RPP5/2BK2R1 b - - 0 8");

    Moves moves = GenerateLegalMoves(&board);
    size_t i;
    forrange(i, 0, moves.count){
        char moveStr[6];
        MoveToString(moves.list[i], moveStr);
        printf("%s\n", moveStr);
    }
}


int main(int argc, char** argv){
    InitZobrist();
    InitMasks();

    UciMain(argc, argv);

    // game(argv[1]);
    
    return 0;
}
