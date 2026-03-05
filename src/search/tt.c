#include <stdlib.h>
#include <string.h>
#include "tt.h"

/* Mate scores are adjusted by ply when storing/retrieving so they remain valid at any ply. */
#define MATE_THRESHOLD 899488

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

static int tt_unadjust_score(int stored, int ply)
{
    if (stored > MATE_THRESHOLD)
        return stored + ply;
    if (stored < -MATE_THRESHOLD)
        return stored - ply;
    return stored;
}

bool tt_probe(uint64_t key,
              int depth,
              int alpha,
              int beta,
              int ply,
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

        int score = tt_unadjust_score(entry->score, ply);

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

static int tt_adjust_score(int score, int ply)
{
    if (score > MATE_THRESHOLD)
        return score - ply;
    if (score < -MATE_THRESHOLD)
        return score + ply;
    return score;
}

void tt_store(uint64_t key,
              int depth,
              int score,
              TTNodeType type,
              Move bestMove,
              int ply)
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
        entry->score = tt_adjust_score(score, ply);
        entry->type = type;
        entry->bestMove = bestMove;
    }
}
