#ifndef TT_H
#define TT_H

#include "castro.h"

typedef enum {
    TT_EXACT,
    TT_LOWERBOUND,
    TT_UPPERBOUND
} TTNodeType;

typedef struct {
    uint64_t key;
    int depth;
    int score;
    TTNodeType type;
    Move bestMove;
} TTEntry;

void tt_init(size_t mb);
void tt_clear(void);

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
