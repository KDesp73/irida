#ifndef EVAL_CACHE_H
#define EVAL_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include "eval.h"

// @module eval_cache
// @desc Eval cache: clear, probe, store, cached eval wrapper.

// @function eval_cache_clear
// @desc Clear the eval cache. Call at the start of each search.
void eval_cache_clear(void);

// @function eval_cache_probe
// @desc Probe cache by Zobrist key. Returns true and sets *out_score if hit.
// @param key Zobrist key.
// @param out_score Output score (set on hit).
// @returns bool True if cache hit.
bool eval_cache_probe(uint64_t key, int* out_score);

// @function eval_cache_store
// @desc Store (key, score) in the cache.
// @param key Zobrist key.
// @param score Score to store.
void eval_cache_store(uint64_t key, int score);

// @function eval_cached
// @desc Evaluate position with cache: probe first, on miss call eval(board) and store result.
// @param board Board to evaluate.
// @param eval Evaluation function.
// @returns int Score (centipawns).
int eval_cached(Board* board, EvalFn eval);

#endif
