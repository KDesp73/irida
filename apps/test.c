#include "castro.h"
#include "eval.h"
#include "moveordering.h"
#include "tt.h"
#define TEST_IMPLEMENTATION
#include "IncludeOnly/test.h"
#include "registry.h"
#include <stdio.h>
#include "uci.h"
#include "search.h"


Engine engine;
UciState uci_state = { .stopRequested = false };

SearchStats g_searchStats = {0};
SearchConfig g_searchConfig = {
    .maxDepth = 20,
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
    castro_InitMagic();
    pesto_init();
    init_mvv_lva();
    tt_init(16);

    test_set_dispatcher(dispatch_test_by_name);

    char* b0[] = { "test_draws" };
    char* b1[] = { "test_eval", "test_eval_breakdown", "test_eval_log", "test_symmetry" };
    char* b2[] = { "test_search" };
    char* b3[] = { "test_eval_stockfish" };

    char** batches[] = { b0, b1, b2, b3 };
    size_t sizes[] = {
        sizeof(b0) / sizeof(char*),
        sizeof(b1) / sizeof(char*),
        sizeof(b2) / sizeof(char*),
        sizeof(b3) / sizeof(char*),
    };

    test_batches(batches, sizeof(batches) / sizeof(char**), sizes);

    return test_run(argc, argv);
}
