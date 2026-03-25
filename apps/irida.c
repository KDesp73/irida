#include "castro.h"
#include "core.h"
#include "eval.h"
#include "moveordering.h"
#include "search.h"
#include "uci.h"

Engine engine;
UciState uci_state;

SearchStats g_searchStats = {0};
SearchConfig g_searchConfig = {
    .maxDepth = 100,
    .timeLimitMs = 0, // No limit unless provided by the gui
    .threads = 1,

    .useAspiration = true,
    .usePVS = true,
    .useLMR = true,
    .useNMP = true,
    .useTT = true,
    .useQuiescence = true,
    .useSyzygy = false,

    .syzygyProbeDepth = 1,
    .syzygyProbeLimit = 4,
    .syzygy50MoveRule = true,
};

int main(void)
{
    EngineInit(&engine);
    engine.eval = nnue_eval;
    engine.search = search;
    engine.order = order_moves;

    UciMain();

    castro_BoardFree(&engine.board);
}
