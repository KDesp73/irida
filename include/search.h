#ifndef SEARCH_H
#define SEARCH_H

#include <stdint.h>
#include <stdbool.h>

#include "castro.h"
#include "eval.h"
#include "moveordering.h"

// @module search
// @desc Search: config, stats, root/PVS/quiescence, timer, and global search state.

// @const INF
// @desc Score constant for infinity (e.g. mate/unbound).
#define INF        1000000

// @const MATE_SCORE
// @desc Score magnitude for mate.
#define MATE_SCORE 900000

// @const MAX_PLY
// @desc Maximum search ply.
#define MAX_PLY    128

// @struct SearchConfig
// @desc Search parameters: depth, time, null-move, LMR, aspiration, TT, quiescence, Syzygy options.
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

// @struct SearchStats
// @desc Counters and diagnostics for the last search (nodes, qnodes, TT hits, selDepth).
typedef struct {
    uint64_t nodes;
    uint64_t qnodes;
    uint64_t ttHits;
    uint64_t rootChildTtHits;  /* diagnostic: TT hits at ply 1 (root children) */
    int selDepth;
} SearchStats;

// @type SearchFn
// @desc Function pointer: root search (board, eval, order, config) -> best Move.
typedef Move (*SearchFn)(Board* board,
                         EvalFn eval,
                         OrderFn order,
                         SearchConfig* config);

// @function search_root
// @desc Root search (iterative deepening entry).
// @param board Board position.
// @param eval Evaluation function.
// @param order Move ordering function.
// @param config Search config.
// @returns Move Best move from root.
Move search_root(Board* board,
                 EvalFn eval,
                 OrderFn order,
                 SearchConfig* config);


// @function search
// @desc Principal Variation Search (PVS).
// @param board Board position.
// @param depth Remaining depth.
// @param alpha Alpha bound.
// @param beta Beta bound.
// @param ply Current ply.
// @param eval Evaluation function.
// @param order Move ordering function.
// @returns int Score (centipawns or mate).
int search(Board* board,
           int depth,
           int alpha,
           int beta,
           int ply,
           EvalFn eval,
           OrderFn order);

// @function quiescence
// @desc Quiescence search (captures only).
// @param board Board position.
// @param alpha Alpha bound.
// @param beta Beta bound.
// @param ply Current ply.
// @param eval Evaluation function.
// @param order Move ordering function.
// @returns int Score.
int quiescence(Board* board,
               int alpha,
               int beta,
               int ply,
               EvalFn eval,
               OrderFn order);


// @function search_start_timer
// @param timeLimitMs Time limit in milliseconds (0 = no limit).
void search_start_timer(int timeLimitMs);

// @function search_time_up
// @returns bool True if time limit exceeded.
bool search_time_up(void);

// @function search_elapsed_ms
// @returns uint64_t Elapsed milliseconds since timer start.
uint64_t search_elapsed_ms(void);

// @var g_searchStats
// @desc Global search statistics for last search.
extern SearchStats g_searchStats;

// @var g_searchConfig
// @desc Global search config (set by UCI go command).
extern SearchConfig g_searchConfig;

#endif // SEARCH_H
