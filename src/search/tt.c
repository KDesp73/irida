/*
 * Theory: Transposition table (replace-by-key, generation-based invalidation).
 *
 * Positions are stored by Zobrist key; the index is key & (size-1) for a
 * power-of-two table. Each search increments a generation counter; only entries
 * with matching generation are used, so we never reuse entries from a previous
 * search. Mate scores are adjusted by ply when storing and unadjusted when
 * probing so they remain valid at any depth. Entries store depth, score type
 * (exact, lower bound, upper bound), and best move for ordering.
 */
#include <stdlib.h>
#include <string.h>
#include "tt.h"

/* Mate scores are adjusted by ply when storing/retrieving so they remain valid at any ply. */
#define MATE_THRESHOLD 899488

static TTEntry* ttTable = NULL;
static size_t ttSize = 0;
static size_t ttMask = 0;
static uint16_t g_tt_generation = 0;

void tt_inc_generation(void)
{
    g_tt_generation++;
}

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

    if (entry->generation != g_tt_generation)
        return false;

    *outMove = entry->bestMove;

    /* Only use score if entry is from a strictly deeper search (fixes ID tree not growing). */
    if (entry->depth > depth) {

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

            case TT_NONE:
                break;
        }
    }

    return false;
}

bool tt_probe_pv(uint64_t key, Move* outMove)
{
    if (!ttTable)
        return false;

    TTEntry* entry = &ttTable[tt_index(key)];

    if (entry->key != key)
        return false;

    if (entry->generation != g_tt_generation)
        return false;

    *outMove = entry->bestMove;
    return true;
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
        entry->generation = g_tt_generation;
    }
}

int tt_hashfull(void)
{
    if (!ttTable || ttSize == 0)
        return 0;
    /* Sample up to 1000 entries to estimate fill (permill 0-1000) */
    size_t step = (ttSize / 1000) ? (ttSize / 1000) : 1;
    size_t count = 0;
    size_t samples = 0;
    for (size_t i = 0; i < ttSize && samples < 1000; i += step, samples++) {
        if (ttTable[i].key != 0 && ttTable[i].generation == g_tt_generation)
            count++;
    }
    return samples ? (int)(count * 1000 / samples) : 0;
}
