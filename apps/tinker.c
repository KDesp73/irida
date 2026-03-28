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

int main(int argc, char** argv)
{
    kv_parse(argc, argv);
    EngineInit(&engine);

    const char* fen = kv_get("fen", NULL);
    int ply = strtol(kv_get("ply", "1"), NULL, 10);

    printf("fen=%s\n", fen);
    printf("ply=%d\n", ply);

    Board board = {0};
    castro_BoardInitFen(&board, fen);
    
    Moves moves = castro_GenerateMoves(&board, MOVE_LEGAL);
    Moves orig = moves;
    order_moves(&board, moves.list, moves.count, ply, NULL_MOVE);

    moves_diff(orig, moves);

    castro_BoardFree(&board);
    return 0;
}

// static char* split_left(char* str) {
//     size_t i = 0;
//     while (str[i] && !isspace((unsigned char)str[i])) i++;
//     if (str[i]) i++;
//     return str + i;
// }
//
// static void wait_enter()
// {
//     printf("Press Enter to continue...");
//     int c;
//     while ((c = getchar()) != '\n' && c != EOF);
// }
//
// int main(int argc, char** argv)
// {
//     EngineInit(&engine);
//
//     char command[] = "position startpos moves e2e4 b8c6 b1c3 e7e6 d2d4 f8b4 d4d5 b4c3 b2c3 d8f6 d5c6 f6c3 d1d2 c3a1 g1f3 d7c6 f1c4 g8f6 e1g1 f6e4 d2b4 a7a5 b4b3 a1f6 c1b2 f6h6 f1e1 e4d6 b2c1 h6h5 c4e2 e8f8 f3d4 h5h4 g2g3 h4d4 c1b2 d4b4 b3b4 a5b4 e1a1 a8a5 a2a3 b4a3 b2a3 f8g8 a3d6 a5a1 g1g2 c7d6 e2f3 e6e5 c2c4 c8e6 c4c5 d6c5 h2h4 a1e1 g3g4 e5e4 f3e4 e1e4 f2f3 e4e1 g2g3 g8f8 g3f4 e6d5 g4g5 e1f1 f4g4 d5f3 g4f5 f8e7 f5f4 h8a8 h4h5 f3h5 f4g3 f1f3 g3h4 a8a4 h4h5 f3h3";
//     // UciHandleCommand(&uci_state, command);
//
//     Game game = {0};
//     castro_GameInit(&game, "tinker", "local", "Player 1", "Player 2", STARTING_FEN);
//
//     castro_BoardInitFen(&engine.board, STARTING_FEN);
//
//     char* token = strtok(command, " ");
//     for (int i = 0; i < 2 && token != NULL; i++) {
//         token = strtok(NULL, " ");
//     }
//
//     // Process each move using strtok
//     while ((token = strtok(NULL, " ")) != NULL) {
//         Move move = castro_StringToMove(token);
//
//         SanMove san = {0};
//         castro_Notate(&engine.board, move, &san);
//         castro_GameAddMove(&game, san);
//         castro_MakeMove(&engine.board, move);
//
//         castro_BoardPrintMove(&engine.board, move);
//         wait_enter();
//         ansi_clear_screen();
//     }
//
//     char pgn_output[4096] = {0};
//     castro_PgnExport(&game, pgn_output);
//     printf("%s\n", pgn_output);
//
//     castro_BoardFree(&engine.board);
//
//     return 0;
// }
