#ifndef SEARCH_H
#define SEARCH_H

#include <stdint.h>
#include <stdbool.h>

#include "castro.h"
#include "eval.h"
#include "moveordering.h"

#define INF        1000000
#define MATE_SCORE 900000
#define MAX_PLY    128

typedef struct {
    int maxDepth;
    int timeLimitMs;      // 0 = no limit
    bool useNullMove;
    bool useLMR;
    bool useAspiration;
    bool useTT;
    bool useQuiescence;
    int syzygyProbeDepth;
    int syzygyProbeLimit;
    bool syzygy50MoveRule;
} SearchConfig;

typedef struct {
    uint64_t nodes;
    uint64_t qnodes;
    uint64_t ttHits;
    uint64_t rootChildTtHits;  /* diagnostic: TT hits at ply 1 (root children) */
    int selDepth;
} SearchStats;

typedef Move (*SearchFn)(Board* board,
                         EvalFn eval,
                         OrderFn order,
                         SearchConfig* config);

/* Root search (iterative deepening entry) */
Move search_root(Board* board,
                 EvalFn eval,
                 OrderFn order,
                 SearchConfig* config);


/* Principal Variation Search (PVS) */
int search(Board* board,
           int depth,
           int alpha,
           int beta,
           int ply,
           EvalFn eval,
           OrderFn order);

/* Quiescence Search */
int quiescence(Board* board,
               int alpha,
               int beta,
               int ply,
               EvalFn eval,
               OrderFn order);


void search_start_timer(int timeLimitMs);
bool search_time_up(void);
uint64_t search_elapsed_ms(void);

extern SearchStats g_searchStats;
extern SearchConfig g_searchConfig;

#endif // SEARCH_H
