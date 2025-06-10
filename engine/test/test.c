#include "masks.h"
#include <string.h>
#define TEST_IMPLEMENTATION
#include "IncludeOnly/test.h"
#include "tests.h"

char* TEST_DIRECTORY = "engine/test";

void load()
{
    LOAD_TEST("test_evaluation_mirroring");
}

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
        RUN_TEST(test_evaluation_mirroring)
    END_TESTS
}

