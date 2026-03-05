#include "castro.h"
#include "eval.h"
#include "moveordering.h"
#include "tt.h"
#define TEST_IMPLEMENTATION
#include "extern/IncludeOnly/test.h"
#include "registry.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    castro_InitMasks();
    pesto_init();
    init_mvv_lva();
    tt_init(16);

    test_set_dispatcher(dispatch_test_by_name);

    char* b0[] = { "test_draws" };
    char* b1[] = { "test_eval" };
    char* b2[] = { "test_search" };

    char** batches[] = { b0, b1, b2 };
    size_t sizes[] = {
        sizeof(b0) / sizeof(char*),
        sizeof(b1) / sizeof(char*),
        sizeof(b2) / sizeof(char*),
    };

    test_batches(batches, sizeof(batches) / sizeof(char**), sizes);

    return test_run(argc, argv);
}
