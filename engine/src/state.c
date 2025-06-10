#include "uci.h"

static const char* safe_str(const char* s) {
    return s ? s : "<null>";
}

void PrintUciOptions(State* state)
{
    for (int i = 0; i < state->uciOptionCount; i++) {
        const UciOption *opt = &state->uciOptions[i];
        const char* def = safe_str(opt->default_value);

        switch (opt->type) {
            case UCI_CHECK:
                LogPrintf("option name %s type check default %s\n", 
                          safe_str(opt->name),
                          (*def == '\0') ? "<empty>" : def);
                break;

            case UCI_SPIN:
                LogPrintf("option name %s type spin default %s min %d max %d\n", 
                          safe_str(opt->name),
                          (*def == '\0') ? "<empty>" : def,
                          opt->params.min, opt->params.max);
                break;

            case UCI_COMBO:
                LogPrintf("option name %s type combo default %s\n", 
                          safe_str(opt->name),
                          (*def == '\0') ? "<empty>" : def);
                for (int j = 0; j < 10 && opt->params.combos[j][0] != '\0'; j++) {
                    LogPrintf("option name %s var %s\n", 
                              safe_str(opt->name),
                              safe_str(opt->params.combos[j]));
                }
                break;

            case UCI_STRING:
                LogPrintf("option name %s type string default %s\n", 
                          safe_str(opt->name),
                          (*def == '\0') ? "<empty>" : def);
                break;
        }
    }
}
