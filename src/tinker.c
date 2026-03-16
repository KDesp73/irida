#include "castro.h"
#include "core.h"
#include "moveordering.h"
#include <stdlib.h>
#include <string.h>
#include "IncludeOnly/kv.h"
#include "IncludeOnly/ansi.h"

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

Engine engine = {0};
SearchStats g_searchStats = {0};
SearchConfig g_searchConfig = {
    .maxDepth = 100,
    .timeLimitMs = 0, // No limit unless provided by the gui
    .useAspiration = true,
    .useLMR = true,
    .useNullMove = true,
    .useTT = true,
    .useQuiescence = true,
    .syzygyProbeDepth = 1,
    .syzygyProbeLimit = 7,
    .syzygy50MoveRule = true,
};

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
