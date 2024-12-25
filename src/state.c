#include "uci.h"

void InitUciOptions()
{
    // Hash option (spin)
    strcpy(state.uciOptions[state.uciOptionCount].name, "Hash");
    state.uciOptions[state.uciOptionCount].type = UCI_SPIN;
    state.uciOptions[state.uciOptionCount].value.spin = 64;
    state.uciOptions[state.uciOptionCount].params.min = 1;
    state.uciOptions[state.uciOptionCount].params.max = 2048;
    strcpy(state.uciOptions[state.uciOptionCount].default_value, "64");
    state.uciOptionCount++;

    // Ponder option (check)
    strcpy(state.uciOptions[state.uciOptionCount].name, "Ponder");
    state.uciOptions[state.uciOptionCount].type = UCI_CHECK;
    state.uciOptions[state.uciOptionCount].value.check = false;
    strcpy(state.uciOptions[state.uciOptionCount].default_value, "false");
    state.uciOptionCount++;
}

void PrintUciOptions()
{
    for (int i = 0; i < state.uciOptionCount; i++) {
        const UciOption *opt = &state.uciOptions[i];
        switch (opt->type) {
            case UCI_CHECK:
                printf("option name %s type check default %s\n", opt->name, opt->default_value);
                break;
            case UCI_SPIN:
                printf("option name %s type spin default %s min %d max %d\n", opt->name, opt->default_value, opt->params.min, opt->params.max);
                break;
            case UCI_COMBO:
                printf("option name %s type combo default %s\n", opt->name, opt->default_value);
                for (int j = 0; j < 10 && opt->params.combos[j][0] != '\0'; j++) {
                    printf("option name %s var %s\n", opt->name, opt->params.combos[j]);
                }
                break;
            case UCI_STRING:
                printf("option name %s type string default %s\n", opt->name, opt->default_value);
                break;
        }
    }
}
