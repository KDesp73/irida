/*
 * test.c
 *
 * test.h's entrypoint
 *
 * Usage: ./check [-n <BATCH-N>]
 *
 * See https://github.com/KDesp73/IncludeOnly/blob/main/libs/test.h
 */

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
    .threads = 1,
    .useAspiration = true,
    .usePVS = true,
    .useLMR = true,
    .useNMP = true,
    .useTT = true,
    .useQuiescence = true,
    .syzygyProbeDepth = 1,
    .syzygyProbeLimit = 7,
    .syzygy50MoveRule = true,
    .useNNUEAccumulator = false,
};



int main(int argc, char** argv)
{
    castro_InitMasks();
    castro_InitMagic();
    irida_EvalPestoInit();
    irida_MVVLVAInit();
    irida_TTInit(16);

    test_set_dispatcher(dispatch_test_by_name);

    char* b0[] = { "test_draws" };
    char* b1[] = { "test_eval", "test_eval_breakdown", "test_eval_log", "test_symmetry" };
    char* b2[] = { "test_search" };
    char* b3[] = { "test_eval_stockfish", "test_nnue_accumulator" };

    char** batches[] = { b0, b1, b2, b3 };
    size_t sizes[] = {
        sizeof(b0) / sizeof(b0[0]),
        sizeof(b1) / sizeof(b1[0]),
        sizeof(b2) / sizeof(b2[0]),
        sizeof(b3) / sizeof(b3[0]),
    };

    test_batches(batches, sizeof(batches) / sizeof(char**), sizes);

    return test_run(argc, argv);
}
