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
#include <string.h>
#include <stdio.h>

#define FEN_BUFFER_SIZE 256

static char g_nnue_path[512];
static bool g_nnue_loaded = false;

extern int nnue_init(const char* filename);
extern int nnue_evaluate_fen(const char* fen);

bool nnue_load(const char* path)
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

bool nnue_available(void)
{
    return g_nnue_loaded;
}

int nnue_eval(Board* board)
{
    // Fallback
    if (!g_nnue_loaded) return evaluation(board);

    char fen[FEN_BUFFER_SIZE];
    castro_FenExport(board, fen);
    int score = nnue_evaluate_fen(fen);
    if (!board->turn)  // black to move
        score = -score;
    return score;
}
