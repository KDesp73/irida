#include "board.h"
#include "evaluation.h"
#include "gui/gui.h"
#include "masks.h"
#include "move.h"
#include "movegen.h"
#include "notation.h"
#include "perft.h"
#include "square.h"
#include "uci.h"
#include "zobrist.h"

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "IncludeOnly/ansi.h"
#include "IncludeOnly/logging.h"
#define MENU_IMPLEMENTATION
#include "IncludeOnly/menu.h"

#define forrange(index, from, to) \
    for(index = from; (from < to) ? i < to : i > to; (from < to) ? i++ : i--)

void perft(size_t depth, const char* fen) {
    Board board;
    BoardInitFen(&board, fen);

    u64 count = Perft(&board, depth, true);

    printf("Nodes reached: %llu\n", count);

    BoardFree(&board);
}

int flush_input()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
    return 0;
}
void BoardInfoPrint(const Board* board)
{
    printf("Turn: %s\n", board->turn ? "White" : "Black");
    printf("Castling rights: %d\n", board->castling_rights);
    char name[3] = "-";
    if(board->enpassant_square != 64)
        SquareToName(name, board->enpassant_square);
    printf("Enpassant: %s\n", name);

    int eval = Evaluation(board);
    printf("Evaluation: %d\n", eval);
}
#define BOARD_PRINT(board) \
    do { \
        ansi_clear_screen(); \
        BoardPrint(&board, 64); \
        BoardInfoPrint(&board); \
    } while(0)

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
        printf("Invalid move format. Use 4 or 5 characters (e.g., e2e4 or h7h8q).\n");
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
    BoardInfoPrint(board);
    return true;
}


int game(const char* fen)
{
    Board board;
    BoardInitFen(&board, fen);
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
            BOARD_PRINT(board);
            UnmakeMove(&board);
            break;
        case 2:
            BoardPrintBitboard(&board, GenerateLegalMovesBitboard(&board));
            BoardInfoPrint(&board);
            break;
        case 3: 
            BoardPrintBitboard(&board, GenerateLegalMovesBitboard(&board));
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

int main(int argc, char** argv){
    InitZobrist();
    InitMasks();

    if (argc >= 2) {
        if (!strcmp(argv[1], "game")) {
            game(argv[2]);
        } else if (!strcmp(argv[1], "perft")) {
            if (argc < 3) {
                ERRO("Please provide the depth");
                exit(1);
            }
            perft(atoi(argv[2]), argv[3]);
        } else if (!strcmp(argv[1], "gui")) {
            gui(argv[2]);
        } else if(!strcmp(argv[1], "eval")) {
            if (argc < 3) {
                ERRO("Please provide the fen");
                exit(1);
            }
            Board board;
            BoardInitFen(&board, argv[2]);
            float eval = (float) Evaluation(&board) / 100;
            printf("%s%.2f\n", (eval > 0) ? "+" : "", eval);
        }

        exit(0);
    }

    UciMain(argc, argv);

    return 0;
}
