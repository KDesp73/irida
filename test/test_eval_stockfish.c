int test_eval_stockfish(const char* fen, int max_cp_diff);

#define TAG "eval_stockfish"
#include "IncludeOnly/test.h"
#include "castro.h"
#include "eval.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

/* Run Stockfish eval on FEN; return centipawns (side-to-move) or 99999 on parse failure / no stockfish. */
static int stockfish_eval_cp(const char* fen, const char* stockfish_path)
{
    char buf[4096];
    FILE* f;
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "echo \"position fen %s\neval\" | \"%s\" 2>/dev/null", fen, stockfish_path);
    f = popen(cmd, "r");
    if (!f)
        return 99999;
    while (fgets(buf, sizeof(buf), f) != NULL) {
        char* p = strstr(buf, "Final evaluation");
        if (p) {
            p += (int)strlen("Final evaluation");
            while (*p == ' ' || *p == '\t') p++;
            double pawns = strtod(p, NULL);
            pclose(f);
            int cp = (int)(pawns * 100.0 + (pawns >= 0 ? 0.5 : -0.5));
            /* FEN " b " = black to move: Stockfish reports white perspective */
            if (strstr(fen, " b ") != NULL)
                cp = -cp;
            return cp;
        }
    }
    pclose(f);
    return 99999;
}

static const char* find_stockfish(void)
{
    const char* path = getenv("STOCKFISH");
    if (path && path[0])
        return path;
    /* PATH search: stockfish or Stockfish */
    if (system("which stockfish >/dev/null 2>&1") == 0)
        return "stockfish";
    if (system("which Stockfish >/dev/null 2>&1") == 0)
        return "Stockfish";
    return NULL;
}

int test_eval_stockfish(const char* fen, int max_cp_diff)
{
    const char* sf = find_stockfish();
    if (!sf) {
        SUCC("Stockfish not in PATH (set STOCKFISH= to run)");
        return 1;
    }
    Board board;
    castro_BoardInitFen(&board, fen);
    int our_cp = irida_Evaluation(&board);
    castro_BoardFree(&board);
    int sf_cp = stockfish_eval_cp(fen, sf);
    if (sf_cp == 99999) {
        FAIL("could not get Stockfish eval (parse or run failed)");
        return 0;
    }
    int diff = our_cp - sf_cp;
    if (diff < 0) diff = -diff;
    if (diff > max_cp_diff) {
        FAIL("eval diff |ours=%d - sf=%d| = %d > max %d", our_cp, sf_cp, diff, max_cp_diff);
        return 0;
    }
    SUCC("ours=%d sf=%d diff=%d (max %d)", our_cp, sf_cp, diff, max_cp_diff);
    return 1;
}
