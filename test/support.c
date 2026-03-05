/* Defines globals required by the engine when linking tests (no main.c). */
#include "search.h"
#include "uci.h"

SearchStats g_searchStats = {0};
SearchConfig g_searchConfig = {
    .maxDepth = 20,
    .timeLimitMs = 0,
    .useAspiration = true,
    .useLMR = true,
    .useNullMove = true,
    .useTT = true,
    .useQuiescence = true,
    .syzygyProbeDepth = 1,
    .syzygyProbeLimit = 7,
    .syzygy50MoveRule = true,
};

UciState uci_state = { .stopRequested = false };
