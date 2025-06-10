#include "IncludeOnly/test.h"
#include "board.h"
#include "evaluation.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
static char mirror_piece(char c);
static void mirror_fen(const char *fen, char *out);

int test_evaluation_mirroring(char* white_fen)
{
    Board white_board = {0};
    BoardInitFen(&white_board, white_fen);
    Eval white_eval = Evaluation(&white_board);
    BoardFree(&white_board);

    char black_fen[256];
    mirror_fen(white_fen, black_fen);

    Board black_board = {0};
    BoardInitFen(&black_board, black_fen);
    Eval black_eval = Evaluation(&black_board);
    BoardFree(&black_board);

    if(white_eval.total != -black_eval.total) {
        FAIL("Mirror eval test failed for %s. White: %d, Black: %d", white_fen, white_eval.total, black_eval.total);
        return 0;
    }

    SUCC("Passed mirror test");
    return 1;
}

static char mirror_piece(char c)
{
    if (isupper(c)) return tolower(c);
    if (islower(c)) return toupper(c);
    return c;
}

static void mirror_fen(const char *fen, char *out)
{
    char board[8][8] = {0};
    int row = 0, col = 0;
    const char *ptr = fen;

    // Parse board section
    while (*ptr && *ptr != ' ') {
        if (*ptr == '/') {
            row++;
            col = 0;
        } else if (isdigit(*ptr)) {
            col += *ptr - '0';
        } else {
            board[row][col++] = *ptr;
        }
        ptr++;
    }

    // Mirror board vertically and swap case
    char mirrored_board[1024] = {0};
    for (int r = 0; r < 8; r++) {
        int empty = 0;
        for (int f = 0; f < 8; f++) {
            char piece = board[7 - r][f];
            if (piece == 0) {
                empty++;
            } else {
                if (empty) {
                    char tmp[8];
                    sprintf(tmp, "%d", empty);
                    strcat(mirrored_board, tmp);
                    empty = 0;
                }
                char mirrored = mirror_piece(piece);
                strncat(mirrored_board, &mirrored, 1);
            }
        }
        if (empty) {
            char tmp[8];
            sprintf(tmp, "%d", empty);
            strcat(mirrored_board, tmp);
        }
        if (r < 7) strcat(mirrored_board, "/");
    }

    // Skip space after board
    while (*ptr == ' ') ptr++;

    // Side to move
    char side = (*ptr == 'w') ? 'b' : 'w';
    ptr++;
    while (*ptr == ' ') ptr++;

    // Castling rights
    char castling[5] = {0};
    int i = 0;
    if (*ptr == '-') {
        castling[0] = '-';
        ptr++;
    } else {
        while (*ptr && *ptr != ' ') {
            char c = *ptr;
            castling[i++] = isupper(c) ? tolower(c) : toupper(c);
            ptr++;
        }
        castling[i] = '\0';
    }

    while (*ptr == ' ') ptr++;

    // En passant square
    char ep[3] = "-";
    if (*ptr != '-') {
        ep[0] = *ptr++;
        char rank = *ptr++;
        ep[1] = (rank == '3') ? '6' : (rank == '6') ? '3' : rank;
        ep[2] = '\0';
    } else {
        ptr++;
    }

    // Copy the rest (halfmove/fullmove)
    while (*ptr == ' ') ptr++;
    char rest[64];
    strncpy(rest, ptr, sizeof(rest));
    rest[sizeof(rest) - 1] = '\0';

    // Compose full mirrored FEN
    sprintf(out, "%s %c %s %s %s", mirrored_board, side, castling, ep, rest);
}

