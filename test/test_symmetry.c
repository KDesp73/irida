#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>

int test_symmetry(const char* fen);

#define TAG "symmetry"
#include "IncludeOnly/test.h"
#include "castro.h"
#include "eval.h"

static void mirror_fen(const char* src, char dst[])
{
    char board_part[128], color, castling[16], en_passant[8];
    int halfmove, fullmove;
    
    sscanf(src, "%s %c %s %s %d %d", board_part, &color, castling, en_passant, &halfmove, &fullmove);

    char* ranks[8];
    char temp_board[128];
    strcpy(temp_board, board_part);
    
    char* token = strtok(temp_board, "/");
    int i = 0;
    while (token != NULL && i < 8) {
        ranks[i++] = token;
        token = strtok(NULL, "/");
    }

    dst[0] = '\0';
    for (int j = 7; j >= 0; j--) {
        strcat(dst, ranks[j]);
        if (j > 0) strcat(dst, "/");
    }

    for (int j = 0; dst[j] != '\0'; j++) {
        if (isupper((unsigned char)dst[j])) dst[j] = tolower((unsigned char)dst[j]);
        else if (islower((unsigned char)dst[j])) dst[j] = toupper((unsigned char)dst[j]);
    }

    char suffix[64];
    sprintf(suffix, " %c %s %s %d %d", 
            (color == 'w' ? 'b' : 'w'), 
            castling, en_passant, halfmove, fullmove);
    strcat(dst, suffix);
}

int test_symmetry(const char* fen)
{
    Board board = {0};
    Board mirrored_board = {0};
    char mirrored_fen_str[256];

    castro_BoardInitFen(&board, fen);
    int eval_orig = irida_evaluation(&board);

    mirror_fen(fen, mirrored_fen_str);
    castro_BoardInitFen(&mirrored_board, mirrored_fen_str);
    int eval_mirrored = irida_evaluation(&mirrored_board);

    if (eval_orig != eval_mirrored) {
        FAIL("Symmetry mismatch! FEN: %s (Eval: %d), Mirrored: %s (Eval: %d)", 
             fen, eval_orig, mirrored_fen_str, eval_mirrored);
        castro_BoardFree(&board);
        castro_BoardFree(&mirrored_board);
        return false;
    }

    castro_BoardFree(&board);
    castro_BoardFree(&mirrored_board);
    
    SUCC("Evaluation symmetry check passed for %s", fen);
    return true;
}
