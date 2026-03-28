#ifndef SHARED_H
#define SHARED_H

#include "core.h"
#include "uci.h"

Engine engine;
UciState uci_state;

SearchStats g_searchStats = {0};
SearchConfig g_searchConfig = {
    .maxDepth = 100,
    .timeLimitMs = 0, // No limit unless provided by the gui
    .threads = 1,

    .useAspiration = true,
    .usePVS = true, // NOTE: enable once move ordering is 100% correct and fast
    .useLMR = false,
    .useNMP = true,
    .useTT = true,
    .useQuiescence = true,
    .useSyzygy = true,

    .syzygyProbeDepth = 1,
    .syzygyProbeLimit = 5,
    .syzygy50MoveRule = true,
};


#endif // SHARED_H
