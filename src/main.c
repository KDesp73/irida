#include "castro.h"
#include "IncludeOnly/logging.h"
#include "core.h"
#include "eval.h"
#include "moveordering.h"
#include "nnue.h"
#include "search.h"
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
    // Load the nnue if possible
    for (size_t i = 0; i < uci_state.uciOptionCount; i++) {
        if (strcmp(uci_state.uciOptions[i].name, "EvalFile") == 0
            && uci_state.uciOptions[i].value.string[0] != '\0') {
            const char* path = uci_state.uciOptions[i].value.string;
            if(!nnue_load(path)){
                ERRO("Could not load nnue %s\n", path);
            }
        }
    }

    EngineInit(&engine, ENGINE_NAME, ENGINE_AUTHOR);
    engine.eval = nnue_eval;
    engine.search = negamax_id_ab_q_mo_tt;
    engine.order = order_moves;


    if(argc > 1) return CliMain(argc, argv);

    UciMain();

    castro_BoardFree(&engine.board);
}
