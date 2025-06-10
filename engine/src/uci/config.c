#include "board.h"
#include "uci.h"

bool GetUciOption(const State* state, char* name, UciOption* opt)
{
    for(size_t i = 0; i < state->uciOptionCount; i++){
        if(!strcmp(name, state->uciOptions[i].name)){
            *opt = state->uciOptions[i];
            return true;
        }
    }
    return false;
}

void LoadUciConfig(State* state)
{
    strncpy(state->startPositionFen, STARTING_FEN, sizeof(state->startPositionFen));
    state->uciMode = true;
    state->debugMode = true;
    state->ponderMode = false;
    state->infiniteMode = false;
    state->depthLimit = 3;
    state->maxBookmoves = 10;

    if(state->uciOptionCount >= MAX_UCI_OPTIONS) return;
    state->uciOptions[state->uciOptionCount++] = (UciOption) {
        .name = "Hash",
        .type = UCI_SPIN,
        .value.spin = 64,
        .params = {
            .min = 1,
            .max = 2048
        },
        .default_value = "64"
    };

    if(state->uciOptionCount >= MAX_UCI_OPTIONS) return;
    state->uciOptions[state->uciOptionCount++] = (UciOption) {
        .name = "Ponder",
        .type = UCI_CHECK,
        .value.check = false,
        .default_value = "false"
    };

    if(state->uciOptionCount >= MAX_UCI_OPTIONS) return;
    state->uciOptions[state->uciOptionCount++] = (UciOption) {
        .name = "Skill Level",
        .type = UCI_SPIN,
        .value.spin = 20,
        .params = {
            .min = 0,
            .max = 20,
        },
        .default_value = "20"
    };

    if(state->uciOptionCount >= MAX_UCI_OPTIONS) return;
    state->uciOptions[state->uciOptionCount++] = (UciOption) {
        .name = "SyzygyPath",
        .type = UCI_STRING,
        .value.string = "",
        .default_value = ""
    };

    if(state->uciOptionCount >= MAX_UCI_OPTIONS) return;
    state->uciOptions[state->uciOptionCount++] = (UciOption) {
        .name = "SyzygyProbeDepth",
        .type = UCI_SPIN,
        .value.spin = 1,
        .params = {
            .min = 1,
            .max = 100
        },
        .default_value = "1"
    };

    if(state->uciOptionCount >= MAX_UCI_OPTIONS) return;
    state->uciOptions[state->uciOptionCount++] = (UciOption) {
        .name = "Syzygy50MoveRule",
        .type = UCI_CHECK,
        .value.check = true,
        .default_value = "true"
    };

    if(state->uciOptionCount >= MAX_UCI_OPTIONS) return;
    state->uciOptions[state->uciOptionCount++] = (UciOption) {
        .name = "SyzygyProbeLimit",
        .type = UCI_SPIN,
        .value.spin = 7,
        .params = {
            .min = 0,
            .max = 7
        },
        .default_value = "7"
    };

    if(state->uciOptionCount >= MAX_UCI_OPTIONS) return;
    state->uciOptions[state->uciOptionCount++] = (UciOption) {
        .name = "DebugInputLogFile",
        .type = UCI_STRING,
        .value.string = "/tmp/in.engine.log",
        .default_value = "/tmp/in.engine.log"
    };

    if(state->uciOptionCount >= MAX_UCI_OPTIONS) return;
    state->uciOptions[state->uciOptionCount++] = (UciOption) {
        .name = "DebugOutputLogFile",
        .type = UCI_STRING,
        .value.string = "/tmp/out.engine.log",
        .default_value = "/tmp/out.engine.log"
    };
}

