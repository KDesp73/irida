/*
 * NNUE (Efficiently Updatable Neural Network) evaluation.
 *
 * The engine loads a neural net from a .nnue file (via nnue-probe). Evaluation
 * is done by exporting the board to FEN and calling the probe; the score is
 * in centipawns from the side-to-move perspective (we negate for black). If no
 * net is loaded or load fails, we fall back to PeSTO. This file is a thin
 * wrapper around the external nnue_init / nnue_evaluate_fen API.
 */
#include "nnue.h"
#include "IncludeOnly/logging.h"
#include "eval.h"
#include "castro.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FEN_BUFFER_SIZE 256

static char g_nnue_path[512];
static bool g_nnue_loaded = false;

extern int nnue_init(const char* filename);
extern int nnue_evaluate_fen(const char* fen);

static bool fen_basic_valid(const char* fen)
{
    if (!fen || fen[0] == '\0')
        return false;

    char placement[96];
    char castling[8];
    char ep[8];
    char side;
    int halfmove = 0;
    int fullmove = 0;

    // FEN: <piece placement> <side to move> <castling> <ep> <halfmove> <fullmove>
    if (sscanf(fen, "%95s %c %7s %7s %d %d", placement, &side, castling, ep, &halfmove, &fullmove) != 6)
        return false;

    if (side != 'w' && side != 'b')
        return false;

    // Validate castling rights: "-" or any subset of KQkq.
    if (castling[0] != '-') {
        size_t n = strlen(castling);
        if (n == 0 || n > 4)
            return false;
        for (size_t i = 0; i < n; i++) {
            char c = castling[i];
            if (c != 'K' && c != 'Q' && c != 'k' && c != 'q')
                return false;
        }
    }

    // Validate en passant: "-" or [a-h][3-6]
    if (ep[0] != '-') {
        if (strlen(ep) != 2)
            return false;
        char file = ep[0];
        char rank = ep[1];
        if (file < 'a' || file > 'h')
            return false;
        if (rank < '3' || rank > '6')
            return false;
    }

    // Validate piece placement: 8 ranks separated by '/'.
    int ranks = 0;
    const char* p = placement;
    int white_kings = 0;
    int black_kings = 0;
    while (*p) {
        int squares_in_rank = 0;
        while (*p && *p != '/') {
            if (*p >= '1' && *p <= '8') {
                squares_in_rank += (*p - '0');
            } else {
                char c = *p;
                if (c != 'p' && c != 'r' && c != 'n' && c != 'b' && c != 'q' && c != 'k' &&
                    c != 'P' && c != 'R' && c != 'N' && c != 'B' && c != 'Q' && c != 'K') {
                    return false;
                }
                if (c == 'K')
                    white_kings++;
                else if (c == 'k')
                    black_kings++;
                squares_in_rank += 1;
            }
            p++;
            if (squares_in_rank > 8)
                return false;
        }
        if (squares_in_rank != 8)
            return false;
        ranks++;
        if (*p == '/')
            p++;
    }
    // NNUE evaluation expects both kings to be present.
    return ranks == 8 && white_kings > 0 && black_kings > 0;
}

bool irida_NNUELoad(const char* path)
{
    if (!path || path[0] == '\0') {
        g_nnue_loaded = false;
        return false;
    }

    char expanded[1024];

    if (path[0] == '~') {
        const char* home = getenv("HOME");
        if (!home) {
            g_nnue_loaded = false;
            return false;
        }

        snprintf(expanded, sizeof(expanded), "%s%s", home, path + 1);
    } else {
        strncpy(expanded, path, sizeof(expanded) - 1);
        expanded[sizeof(expanded) - 1] = '\0';
    }

    strncpy(g_nnue_path, expanded, sizeof(g_nnue_path) - 1);
    g_nnue_path[sizeof(g_nnue_path) - 1] = '\0';

    if (nnue_init(g_nnue_path) != 0) {
        g_nnue_loaded = false;
        return false;
    }

    g_nnue_loaded = true;
    printf("info NNUE %s loaded\n", g_nnue_path);
    return true;
}

bool irida_NNUEAvailable(void)
{
    return g_nnue_loaded;
}

int irida_EvalNNUE(Board* board)
{
    // Fallback
    if (!g_nnue_loaded) return irida_Evaluation(board);

    char fen[FEN_BUFFER_SIZE];
    fen[0] = '\0';
    castro_FenExport(board, fen);
    fen[FEN_BUFFER_SIZE - 1] = '\0';

    // The NNUE probe is a black box and has been crashing on malformed inputs.
    // If the exported FEN is malformed, fall back to the PeSTO evaluator.
    if (!fen_basic_valid(fen))
        return irida_Evaluation(board);

    int score = nnue_evaluate_fen(fen);
    if (!board->turn)  // black to move
        score = -score;
    return score;
}
