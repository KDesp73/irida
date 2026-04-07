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
    .contempt = -10,

    .useAspiration = true,
    .usePVS = true,
    .useLMR = true,
    .useNMP = true,
    .useTT = true,
    .useQuiescence = true,
    .useSyzygy = true,

    .syzygyProbeDepth = 1,
    .syzygyProbeLimit = 4,
    .syzygy50MoveRule = true,
};


#endif // SHARED_H
