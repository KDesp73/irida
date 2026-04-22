
#ifndef TEST_REGISTRY_H
#define TEST_REGISTRY_H

#include <string.h>
#include "IncludeOnly/test.h"

#include "test_eval_breakdown.h"
#include "test_eval.h"
#include "test_eval_log.h"
#include "test_eval_stockfish.h"
#include "test_draws.h"
#include "test_symmetry.h"
#include "test_search.h"
#include "test_nnue_accumulator.h"

// Auto-generated dispatcher
static int dispatch_test_by_name(const char* name) {
    if (strcmp(name, "test_eval_breakdown") == 0) return test(TEST_test_eval_breakdown, END);
    if (strcmp(name, "test_eval") == 0) return test(TEST_test_eval, END);
    if (strcmp(name, "test_eval_log") == 0) return test(TEST_test_eval_log, END);
    if (strcmp(name, "test_eval_stockfish") == 0) return test(TEST_test_eval_stockfish, END);
    if (strcmp(name, "test_draws") == 0) return test(TEST_test_draws, END);
    if (strcmp(name, "test_symmetry") == 0) return test(TEST_test_symmetry, END);
    if (strcmp(name, "test_search") == 0) return test(TEST_test_search, END);
    if (strcmp(name, "test_nnue_accumulator") == 0) return test(TEST_test_nnue_accumulator, END);
    return 1;
}

#endif