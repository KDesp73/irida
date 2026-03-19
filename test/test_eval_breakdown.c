int test_eval_breakdown(const char* fen);

#define TAG "eval_breakdown"
#include "IncludeOnly/test.h"
#include "castro.h"
#include "eval.h"
#include <string.h>

int test_eval_breakdown(const char* fen)
{
    Board board;
    castro_BoardInitFen(&board, fen);
    int score = pesto_eval(&board);
    EvalBreakdown b;
    int score2 = eval_breakdown(&board, &b);
    castro_BoardFree(&board);
    if (score != score2) {
        FAIL("pesto_eval %d != pesto_eval_breakdown %d", score, score2);
        return 0;
    }
    int sum = b.material_pst + b.pawn_structure + b.mobility + b.king_safety
            + b.piece_activity + b.space + b.threats + b.endgame;
    if (sum != b.total) {
        FAIL("breakdown sum %d != total %d", sum, b.total);
        return 0;
    }
    if (b.total != score) {
        FAIL("breakdown total %d != pesto_eval %d", b.total, score);
        return 0;
    }
    SUCC("breakdown sum ok (total=%d)", score);
    return 1;
}
