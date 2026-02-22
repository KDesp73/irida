#include <stdlib.h>
#include <string.h>
#include "tt.h"

#define TT_CLUSTER_SIZE 1

static TTEntry* ttTable = NULL;
static size_t ttSize = 0;
static size_t ttMask = 0;


static inline size_t tt_index(uint64_t key) {
    return key & ttMask;
}

void tt_init(size_t mb)
{
    if (ttTable)
        free(ttTable);

    size_t bytes = mb * 1024ULL * 1024ULL;
    size_t entries = bytes / sizeof(TTEntry);

    // Force power of two
    size_t pow2 = 1;
    while (pow2 < entries)
        pow2 <<= 1;

    ttSize = pow2;
    ttMask = ttSize - 1;

    ttTable = (TTEntry*)malloc(ttSize * sizeof(TTEntry));
    memset(ttTable, 0, ttSize * sizeof(TTEntry));
}

void tt_clear(void)
{
    if (ttTable)
        memset(ttTable, 0, ttSize * sizeof(TTEntry));
}

bool tt_probe(uint64_t key,
              int depth,
              int alpha,
              int beta,
              int* outScore,
              Move* outMove)
{
    if (!ttTable)
        return false;

    TTEntry* entry = &ttTable[tt_index(key)];

    if (entry->key != key)
        return false;

    *outMove = entry->bestMove;

    if (entry->depth >= depth) {

        int score = entry->score;

        switch (entry->type) {

            case TT_EXACT:
                *outScore = score;
                return true;

            case TT_LOWERBOUND:
                if (score >= beta) {
                    *outScore = score;
                    return true;
                }
                break;

            case TT_UPPERBOUND:
                if (score <= alpha) {
                    *outScore = score;
                    return true;
                }
                break;
        }
    }

    return false;
}

void tt_store(uint64_t key,
              int depth,
              int score,
              TTNodeType type,
              Move bestMove)
{
    if (!ttTable)
        return;

    TTEntry* entry = &ttTable[tt_index(key)];

    // Replace if:
    // 1. Slot empty
    // 2. Deeper search
    // 3. Same key
    if (entry->key == 0 ||
        depth >= entry->depth ||
        entry->key == key)
    {
        entry->key = key;
        entry->depth = depth;
        entry->score = score;
        entry->type = type;
        entry->bestMove = bestMove;
    }
}
