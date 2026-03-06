#ifndef TT_H
#define TT_H

#include "castro.h"

typedef enum {
    TT_EXACT,
    TT_LOWERBOUND,
    TT_UPPERBOUND,
    TT_NONE
} TTNodeType;

typedef struct {
    uint64_t key;
    int depth;
    int score;
    TTNodeType type;
    Move bestMove;
    uint16_t generation;  /* search generation: only use entry if matches current */
} TTEntry;

void tt_init(size_t mb);
void tt_clear(void);
void tt_inc_generation(void);  /* call at start of each search_root */

bool tt_probe(uint64_t key,
              int depth,
              int alpha,
              int beta,
              int ply,
              int* outScore,
              Move* outMove);

void tt_store(uint64_t key,
              int depth,
              int score,
              TTNodeType type,
              Move bestMove,
              int ply);


#endif // TT_H
