#include "masks.h"
#include <string.h>
#define TEST_IMPLEMENTATION
#include "IncludeOnly/test.h"
#include "tests.h"

char* TEST_DIRECTORY = "engine/test";

void load()
{
    LOAD_TEST("test_evaluation_mirroring");
    LOAD_TEST("test_evaluation_known_position");
    LOAD_TEST("test_evaluation_gradient");
}

/*
 * | Test                | Purpose                   |
 * | ------------------- | ------------------------- |
 * | Mirror Test         | Color symmetry            |
 * | Known Position Eval | Sanity check              |
 * | Gradient Test       | Term logic                |
 * | Differential Eval   | Specific term correctness |
 * | Static vs Result    | High-level accuracy       |
 * | Distribution Check  | Stability and balance     |
 */

int main(int argc, char** argv)
{
    InitMasks();

    if(argc >= 2) {
        if(!strcmp(argv[1], "load")){
            load();
        }
        return 0;
    }

    START_TESTS
        RUN_TEST(test_evaluation_gradient),
        RUN_TEST(test_evaluation_known_position),
        RUN_TEST(test_evaluation_mirroring),
        1
    END_TESTS
}

