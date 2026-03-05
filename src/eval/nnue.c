#include "nnue.h"
#include "eval.h"
#include "castro.h"
#include <string.h>
#include <stdio.h>

#define FEN_BUFFER_SIZE 256

static char g_nnue_path[512];
static bool g_nnue_loaded = false;

#ifdef USE_NNUE_PROBE
extern int nnue_init(const char* filename);
extern int nnue_evaluate_fen(const char* fen);
#endif

bool nnue_load(const char* path)
{
    if (!path || path[0] == '\0') {
        g_nnue_loaded = false;
        return false;
    }

    strncpy(g_nnue_path, path, sizeof(g_nnue_path) - 1);
    g_nnue_path[sizeof(g_nnue_path) - 1] = '\0';

#ifdef USE_NNUE_PROBE
    if (nnue_init(g_nnue_path) != 0) {
        g_nnue_loaded = false;
        return false;
    }
    g_nnue_loaded = true;
    return true;
#else
    (void)path;
    g_nnue_loaded = false;
    return false;
#endif
}

bool nnue_available(void)
{
    return g_nnue_loaded;
}

int nnue_eval(Board* board)
{
#ifdef USE_NNUE_PROBE
    if (g_nnue_loaded) {
        char fen[FEN_BUFFER_SIZE];
        castro_FenExport(board, fen);
        int score = nnue_evaluate_fen(fen);
        /* NNUE typically returns score for white's perspective; adjust for side to move */
        if (!board->turn)  /* black to move */
            score = -score;
        return score;
    }
#endif
    return pesto_eval(board);
}
