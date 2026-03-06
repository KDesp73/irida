
#ifndef TEST_REGISTRY_H
#define TEST_REGISTRY_H

#include <string.h>
#include "IncludeOnly/test.h"

#include "test_eval.h"
#include "test_draws.h"
#include "test_search.h"

// Auto-generated dispatcher
static int dispatch_test_by_name(const char* name) {
    if (strcmp(name, "test_eval") == 0) return test(TEST_test_eval, END);
    if (strcmp(name, "test_draws") == 0) return test(TEST_test_draws, END);
    if (strcmp(name, "test_search") == 0) return test(TEST_test_search, END);
    return 1;
}

#endif