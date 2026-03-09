#include "IncludeOnly/logging.h"
#include "cli.h"
#include "castro.h"
#include "core.h"
#include "eval.h"
#include "nnue.h"
#include "search.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool perft_handler(Context context)
{
    char* fen = context.fen;
    size_t depth = context.depth;

    if(!depth) {
        ERRO("Provide a depth");
        return false;
    }

    Board board = {0};
    castro_BoardInitFen(&board, fen);

    u64 moves = castro_Perft(&board, depth, false);

    printf("%llu\n", moves);

    castro_BoardFree(&board);

    return true;
}

EvalFn eval_dispatcher(char* eval) {
    if(!eval) return engine.eval;
    if(!strcmp(eval, "material")) return material_eval;
    if(!strcmp(eval, "pesto")) return pesto_eval;
    if(!strcmp(eval, "nnue")) return nnue_eval;
    return engine.eval;
}

SearchFn search_dispatcher(char* search) {
    if(!search) return engine.search;
    // TODO: add variants
    return engine.search;
}

bool eval_handler(Context context)
{
    engine.eval = eval_dispatcher(context.eval);

    if (strcmp("nnue", context.eval) == 0) {
        nnue_load(context.nnue_path);
    }

    castro_BoardInitFen(&engine.board, context.fen);
    int eval = engine.eval(&engine.board);

    castro_BoardFree(&engine.board);

    printf("%.1f\n", (double) eval / 100);
    return true;
}

#define BATCH_LINE_MAX 4096

/* Read stdin: repeat [ N (count) ; params line (10 ints mg0..mg4 eg0..eg4) ; N FENs ]. Output N scores (white cp) per batch. */
bool eval_batch_handler(Context context)
{
    (void)context;
    pesto_init();

    char line[BATCH_LINE_MAX];
    int mg[6], eg[6];
    mg[5] = eg[5] = 0;

    while (fgets(line, (int)sizeof(line), stdin) != NULL) {
        int n = atoi(line);
        if (n <= 0)
            break;
        if (fgets(line, (int)sizeof(line), stdin) == NULL)
            break;
        int nread = sscanf(line, "%d %d %d %d %d %d %d %d %d %d",
                           &mg[0], &mg[1], &mg[2], &mg[3], &mg[4],
                           &eg[0], &eg[1], &eg[2], &eg[3], &eg[4]);
        if (nread != 10)
            break;
        pesto_set_tune_values(mg, eg);

        for (int i = 0; i < n; i++) {
            if (fgets(line, (int)sizeof(line), stdin) == NULL)
                break;
            size_t len = strlen(line);
            while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
                line[--len] = '\0';
            Board board = {0};
            castro_BoardInitFen(&board, line);
            int score = pesto_material_pst_eval_white(&board);
            castro_BoardFree(&board);
            printf("%d\n", score);
        }
    }
    return true;
}

/* Read stdin: N (count) ; N FENs. Output N lines of 8 integers (white-perspective: material_pst pawn_structure mobility king_safety piece_activity space threats endgame). */
bool eval_breakdown_batch_handler(Context context)
{
    (void)context;
    pesto_init();

    char line[BATCH_LINE_MAX];
    if (fgets(line, (int)sizeof(line), stdin) == NULL)
        return true;
    int n = atoi(line);
    if (n <= 0)
        return true;

    for (int i = 0; i < n; i++) {
        if (fgets(line, (int)sizeof(line), stdin) == NULL)
            break;
        size_t len = strlen(line);
        while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            line[--len] = '\0';
        Board board = {0};
        castro_BoardInitFen(&board, line);
        EvalBreakdown b;
        (void)pesto_eval_breakdown(&board, &b);
        int sign = board.turn ? 1 : -1;
        printf("%d %d %d %d %d %d %d %d\n",
               sign * b.material_pst, sign * b.pawn_structure, sign * b.mobility,
               sign * b.king_safety, sign * b.piece_activity, sign * b.space,
               sign * b.threats, sign * b.endgame);
        castro_BoardFree(&board);
    }
    return true;
}

bool search_handler(Context context)
{
    engine.eval = eval_dispatcher(context.eval);
    engine.search = search_dispatcher(context.search);

    if(!context.depth) {
        ERRO("Provide a depth");
        return false;
    }

    castro_BoardInitFen(&engine.board, context.fen);

    char bestmove[16];
    Move move = engine.search(&engine.board, engine.eval, engine.order, &g_searchConfig);

    castro_BoardFree(&engine.board);

    castro_MoveToString(move, bestmove);
    printf("%s\n", bestmove);

    return true;
}
