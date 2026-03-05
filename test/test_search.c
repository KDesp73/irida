int test_search(const char* fen, int depth_limit);

#define TAG "search"
#include "IncludeOnly/test.h"
#include "castro.h"
#include "core.h"
#include "search.h"
#include "tt.h"
#include "moveordering.h"
#include <string.h>

int test_search(const char* fen, int depth_limit)
{
    Board board;
    castro_BoardInitFen(&board, fen);
    SearchConfig config = {
        .maxDepth = depth_limit,
        .timeLimitMs = 100,
        .useNullMove = true,
        .useLMR = true,
        .useAspiration = true,
        .useTT = true,
        .useQuiescence = true,
        .syzygyProbeDepth = 1,
        .syzygyProbeLimit = 7,
        .syzygy50MoveRule = true,
    };
    Move move = search_root(&board, pesto_eval, order_moves, &config);
    castro_BoardFree(&board);
    (void)move;
    SUCC("search_root depth %d completed", depth_limit);
    return 1;
}
