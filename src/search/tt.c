/*
 * Transposition table (replace-by-key, generation-based invalidation).
 *
 * Positions are stored by Zobrist key; the index is key & (size-1) for a
 * power-of-two table. Each search increments a generation counter; only entries
 * with matching generation are used, so we never reuse entries from a previous
 * search. Mate scores are adjusted by ply when storing and unadjusted when
 * probing so they remain valid at any depth. Entries store depth, score type
 * (exact, lower bound, upper bound), and best move for ordering.
 *
 * Probes: we may still return bestMove for move ordering when stored depth is
 * below the requested depth (score cutoff only when depth is sufficient).
 * Replacement favors deeper/newer/same-key updates per the policy in tt_store.
 */
#include <stdlib.h>
#include <string.h>
#include "tt.h"

#define MATE_THRESHOLD 899488

static TTEntry* ttTable = NULL;
static size_t ttSize = 0;
static size_t ttMask = 0;
static uint16_t g_tt_generation = 0;

void tt_inc_generation(void) { g_tt_generation++; }

static inline size_t tt_index(uint64_t key) { return key & ttMask; }

void tt_init(size_t mb) {
    if (ttTable) free(ttTable);
    size_t bytes = mb * 1024ULL * 1024ULL;
    size_t entries = bytes / sizeof(TTEntry);
    size_t pow2 = 1;
    while (pow2 < entries) pow2 <<= 1;
    ttSize = pow2;
    ttMask = ttSize - 1;
    ttTable = (TTEntry*)malloc(ttSize * sizeof(TTEntry));
    memset(ttTable, 0, ttSize * sizeof(TTEntry));
}

void tt_clear(void)
{
    if (ttTable && ttSize > 0) {
        // Zero out the entire memory block allocated for the table
        memset(ttTable, 0, ttSize * sizeof(TTEntry));
    }
}

bool tt_probe(uint64_t key, int depth, int alpha, int beta, int ply, int* outScore, Move* outMove) {
    if (!ttTable) return false;

    TTEntry* entry = &ttTable[tt_index(key)];
    if (entry->key != key) return false;

    // We ALWAYS want the bestMove for move ordering, even if depth is insufficient
    *outMove = entry->bestMove;

    if (entry->depth >= depth) {
        int score = tt_unadjust_score(entry->score, ply);

        if (entry->type == TT_EXACT) {
            *outScore = score;
            return true;
        }
        if (entry->type == TT_LOWERBOUND && score >= beta) {
            *outScore = score;
            return true;
        }
        if (entry->type == TT_UPPERBOUND && score <= alpha) {
            *outScore = score;
            return true;
        }
    }
    return false;
}

void tt_store(uint64_t key, int depth, int score, TTNodeType type, Move bestMove, int ply) {
    if (!ttTable) return;

    TTEntry* entry = &ttTable[tt_index(key)];

    /* * Replacement Strategy:
     * Replace if:
     * 1. The slot is empty.
     * 2. This is the same position (update with new depth/score).
     * 3. The new search is deeper than the old one.
     * 4. The old entry is from a previous search generation (stale data).
     */
    bool replace = (entry->key == 0) || 
                   (entry->key == key) || 
                   (depth >= entry->depth) || 
                   (entry->generation != g_tt_generation);

    if (replace) {
        // If we don't have a new best move, keep the old one (useful for Alpha/Beta bounds)
        Move move_to_store = (bestMove != NULL_MOVE) ? bestMove : entry->bestMove;

        entry->key = key;
        entry->depth = depth;
        entry->score = tt_adjust_score(score, ply);
        entry->type = type;
        entry->bestMove = move_to_store;
        entry->generation = g_tt_generation;
    }
}

int tt_adjust_score(int score, int ply)
{
    // If the score is a mate score (very high or very low)
    if (score > MATE_THRESHOLD) {
        return score + ply;
    }
    if (score < -MATE_THRESHOLD) {
        return score - ply;
    }
    
    // Normal evaluation score, no adjustment needed
    return score;
}

int tt_unadjust_score(int stored, int ply)
{
    // If the stored value indicates a mate
    if (stored > MATE_THRESHOLD) {
        return stored - ply;
    }
    if (stored < -MATE_THRESHOLD) {
        return stored + ply;
    }

    // Normal evaluation score
    return stored;
}

int tt_hashfull(void)
{
    // If the table isn't initialized, it's obviously empty
    if (!ttTable || ttSize == 0) {
        return 0;
    }

    /* * We sample the first 1000 entries of the table.
     * This is an industry standard estimate that doesn't 
     * impact search performance significantly.
     */
    size_t sampled_entries = (ttSize < 1000) ? ttSize : 1000;
    size_t occupied_count = 0;

    for (size_t i = 0; i < sampled_entries; i++) {
        // A slot is "full" if it has a key and matches the current search generation
        if (ttTable[i].key != 0 && ttTable[i].generation == g_tt_generation) {
            occupied_count++;
        }
    }

    // Scale the count to a value between 0 and 1000
    return (int)((occupied_count * 1000) / sampled_entries);
}

/**
 * A simplified probe that only retrieves the best move.
 * Used for PV extraction and reporting to the GUI.
 */
bool tt_probe_pv(uint64_t key, Move* outMove)
{
    if (!ttTable)
        return false;

    TTEntry* entry = &ttTable[tt_index(key)];

    // We must ensure the key matches exactly (avoid hash collisions)
    if (entry->key != key)
        return false;

    // Check if the entry is valid for the current search
    if (entry->generation != g_tt_generation)
        return false;

    // If there is no move stored (e.g., an Alpha-node fail-low), return false
    if (entry->bestMove == NULL_MOVE)
        return false;

    *outMove = entry->bestMove;
    return true;
}
