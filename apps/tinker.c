/*
 * tinker.c
 *
 * This file is meant for quick experimentation and testing.
 * It will and should change frequently
 * 
 * NOTE: remove once the engine reaches 1.0.0
 */

#include "castro.h"
#include "core.h"
#include "moveordering.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "IncludeOnly/ansi.h"
#include "IncludeOnly/kv.h"
#include "uci.h"

void print_moves(Moves moves)
{
    char buf[12];
    for(size_t i = 0; i < moves.count; i++) {
        castro_MoveToString(moves.list[i], buf);
        printf("%s\n", buf);
    }
}

void moves_diff(Moves m1, Moves m2) {
    size_t min_size = (m1.count < m2.count) ? m1.count : m2.count;
    char buf1[12], buf2[12];
    for(size_t i = 0; i < min_size; ++i) {
        castro_MoveToString(m1.list[i], buf1);
        castro_MoveToString(m2.list[i], buf2);

        if(strncmp(buf1, buf2, 12) == 0) printf("%s== ", ANSI_GREEN);
        if(strncmp(buf1, buf2, 12) != 0) printf("%s!= ", ANSI_RED);
        printf("%s %s%s\n", buf1, buf2, ANSI_RESET);
    }

    if(m1.count != m2.count) {
        Moves bigger = (m1.count > m2.count) ? m1 : m2;
        char buf[12];
        for(size_t i = min_size; i < bigger.count; ++i) {
            castro_MoveToString(bigger.list[i], buf);
            printf("++ %s\n", buf);
        }
    }
}

#include "shared.h"

// int main(int argc, char** argv)
// {
//     kv_parse(argc, argv);
//     EngineInit(&engine);
//
//     const char* fen = kv_get("fen", NULL);
//     int ply = strtol(kv_get("ply", "1"), NULL, 10);
//
//     printf("fen=%s\n", fen);
//     printf("ply=%d\n", ply);
//
//     Board board = {0};
//     castro_BoardInitFen(&board, fen);
//     
//     Moves moves = castro_GenerateMoves(&board, MOVE_LEGAL);
//     Moves orig = moves;
//     order_moves(&board, moves.list, moves.count, ply, NULL_MOVE);
//
//     moves_diff(orig, moves);
//
//     castro_BoardFree(&board);
//     return 0;
// }

static char* split_left(char* str) {
    size_t i = 0;
    while (str[i] && !isspace((unsigned char)str[i])) i++;
    if (str[i]) i++;
    return str + i;
}

static void wait_enter()
{
    printf("Press Enter to continue...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main(int argc, char** argv)
{
    EngineInit(&engine);

    char command[] = "position startpos moves g1f3 g8f6 c2c4 g7g6 b1c3 f8g7 e2e4 d7d6 d2d4 e8g8 f1e2 e7e5 c1e3 c7c6 d4d5 b8a6 e1g1 f6g4 e3g5 f7f6 g5h4 c6c5 f3e1 h7h5 a2a3 d8e7 b2b3 a6c7 g1h1 g4h6 e2d3 g6g5 h4g5 f6g5 d1h5 c8g4 f2f3 a7a5 h5g4 c7d5 c4d5 f8f3 e1f3 a5a4 g4g5 a4b3 g2g3 b3b2 g5g6 b2a1q g6g5 a8a7 g5e5 a1f1 f3g1 f1g1 h1g1 e7h4 g3h4 d6e5 h2h3 a7a4 d3c4 h6g4 h4h5 g7h6 g1h1 a4a3 h3g4 a3b3 d5d6 g8g7 g4g5 b7b5 c3d5 b3d3 g5g6 b5b4 c4d3 h6d2 h5h6 d2h6 d6d7 b4b3 d7d8r b3b2 d8d7 g7g6 d7d6 g6g5 h1g2 b2b1q d6h6 b1b3 h6f6 c5c4 f6f5 g5g6 d3e2 b3b1 d5c3 b1c2 f5e5 c2c3 e2d1 c3a1 e5d5 c4c3 e4e5 a1b2 g2f3 g6f5 e5e6 f5e6 d5h5 b2b1 h5b5 b1d1";
    // UciHandleCommand(&uci_state, command);

    Game game = {0};
    castro_GameInit(&game, "tinker", "local", "Player 1", "Player 2", STARTING_FEN);

    castro_BoardInitFen(&engine.board, STARTING_FEN);

    char* token = strtok(command, " ");
    for (int i = 0; i < 2 && token != NULL; i++) {
        token = strtok(NULL, " ");
    }

    // Process each move using strtok
    while ((token = strtok(NULL, " ")) != NULL) {
        Move move = castro_StringToMove(token);

        SanMove san = {0};
        castro_Notate(&engine.board, move, &san);
        castro_GameAddMove(&game, san);
        castro_MakeMove(&engine.board, move);

        castro_BoardPrintMove(&engine.board, move);
        wait_enter();
        ansi_clear_screen();
    }

    char pgn_output[4096] = {0};
    castro_PgnExport(&game, pgn_output);
    printf("%s\n", pgn_output);

    castro_BoardFree(&engine.board);

    return 0;
}
