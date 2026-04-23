#ifndef SHARED_H
#define SHARED_H

#include "core.h"
#include "uci.h"

#ifdef ENABLE_ALL
#define ON_ALL true
#else
#define ON_ALL false
#endif

#ifdef ENABLE_ASPIRATION
#define ON_ASPIRATION true
#else
#define ON_ASPIRATION false
#endif

#ifdef ENABLE_PVS
#define ON_PVS true
#else
#define ON_PVS false
#endif

#ifdef ENABLE_LMR
#define ON_LMR true
#else
#define ON_LMR false
#endif

#ifdef ENABLE_NMP
#define ON_NMP true
#else
#define ON_NMP false
#endif

#ifdef ENABLE_TT
#define ON_TT true
#else
#define ON_TT false
#endif

#ifdef ENABLE_QUIESCENCE
#define ON_QUIESCENCE true
#else
#define ON_QUIESCENCE false
#endif

#ifdef ENABLE_SYZYGY
#define ON_SYZYGY true
#else
#define ON_SYZYGY false
#endif

Engine engine;
UciState uci_state;

SearchStats g_searchStats = {0};
SearchConfig g_searchConfig = {
    .maxDepth = 100,
    .timeLimitMs = 0, // No limit unless provided by the gui
    .threads = 1,

    .useAspiration = ON_ALL || ON_ASPIRATION,
    .usePVS        = ON_ALL || ON_PVS,
    .useLMR        = ON_ALL || ON_LMR,
    .useNMP        = ON_ALL || ON_NMP,
    .useTT         = ON_ALL || ON_TT,
    .useQuiescence = ON_ALL || ON_QUIESCENCE,
    .useSyzygy     = ON_ALL || ON_SYZYGY,

    .syzygyProbeDepth = 1,
    .syzygyProbeLimit = 4,
    .syzygy50MoveRule = true,
    .useNNUEAccumulator = true,
};


#endif // SHARED_H
