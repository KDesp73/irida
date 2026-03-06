#ifndef EVAL_CACHE_H
#define EVAL_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include "eval.h"

/** Clear the eval cache. Call at the start of each search so entries are not reused across searches. */
void eval_cache_clear(void);

/** Probe cache by Zobrist key. Returns true and sets *out_score if hit. */
bool eval_cache_probe(uint64_t key, int* out_score);

/** Store (key, score) in the cache. */
void eval_cache_store(uint64_t key, int score);

/** Evaluate position with cache: probe first, on miss call eval(board) and store result. */
int eval_cached(Board* board, EvalFn eval);

#endif
