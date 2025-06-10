#include "board.h"
#include "uci.h"

void LoadUciConfig(State* state)
{
    strncpy(state->startPositionFen, STARTING_FEN, sizeof(state->startPositionFen));
    state->uciMode = true;
    state->debugMode = true;
    state->ponderMode = false;
    state->infiniteMode = false;
    state->depthLimit = 2;
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
}

