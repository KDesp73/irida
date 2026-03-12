#include "castro.h"
#include "core.h"
#include "eval.h"
#include "moveordering.h"
#include "nnue.h"
#include "search.h"
#include "tt.h"
#include "uci.h"
#include "cli.h"
#include "version.h"

Engine engine;
UciState uci_state;

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
    castro_InitMasks();
    pesto_init();
    init_mvv_lva();
    tt_init(16);

    EngineInit(&engine, ENGINE_NAME, "KDesp73");
    engine.eval = nnue_eval;
    engine.search = search_root;
    engine.order = order_moves;

    if(argc > 1) return CliMain(argc, argv);

    UciMain();

    castro_BoardFree(&engine.board);
}
