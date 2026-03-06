#include "eval_cache.h"
#include "castro.h"
#include <string.h>

#define EVAL_CACHE_SIZE 65536
#define EVAL_CACHE_MASK (EVAL_CACHE_SIZE - 1)

typedef struct {
    uint64_t key;
    int score;
} EvalCacheEntry;

static EvalCacheEntry g_eval_cache[EVAL_CACHE_SIZE];

void eval_cache_clear(void)
{
    memset(g_eval_cache, 0, sizeof(g_eval_cache));
}

bool eval_cache_probe(uint64_t key, int* out_score)
{
    size_t i = key & EVAL_CACHE_MASK;
    if (g_eval_cache[i].key == key) {
        *out_score = g_eval_cache[i].score;
        return true;
    }
    return false;
}

void eval_cache_store(uint64_t key, int score)
{
    size_t i = key & EVAL_CACHE_MASK;
    g_eval_cache[i].key = key;
    g_eval_cache[i].score = score;
}

int eval_cached(Board* board, EvalFn eval)
{
    uint64_t key = castro_CalculateZobristHash(board);
    int score;
    if (eval_cache_probe(key, &score))
        return score;
    score = eval(board);
    eval_cache_store(key, score);
    return score;
}
