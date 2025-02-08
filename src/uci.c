#include "uci.h"
#include "move.h"
#include "perft.h"
#include "search.h"
#include "extern/ansi.h"
#include <stdio.h>
#include <string.h>
#define LUAMAN_IMPLEMENTATION
#include "extern/luaman.h"

int UciMain(int argc, char** argv)
{
    char input[1024];
    State state = {0};
    InitState(&state);
    LoadUciConfig(&state);
    StatePrint(&state);

    printf("Welcome to %s%s%s by %s%s%s\n", ANSI_RED, ENGINE_NAME, ANSI_RESET, ANSI_BLUE, ENGINE_AUTHOR, ANSI_RESET);
    FLUSH;
    
    for(;;) {
        if(fgets(input, sizeof(input), stdin) == NULL) continue; 

        input[strcspn(input, "\n")] = 0;

        HandleCommand(&state, input);
    }

    return 0;
}

void setoption(State* state, const char *command)
{
    char option_name[64];
    char option_value[128];

    if (sscanf(command, "setoption name %63s value %127[^\n]", option_name, option_value) != 2) {
        printf("info string Invalid setoption format\n");
        return;
    }

    for (size_t i = 0; i < state->uciOptionCount; i++) {
        if (strcmp(state->uciOptions[i].name, option_name) == 0) {
            switch (state->uciOptions[i].type) {
                case UCI_CHECK:
                    state->uciOptions[i].value.check = (strcmp(option_value, "true") == 0);
                    break;
                case UCI_SPIN:
                    state->uciOptions[i].value.spin = atoi(option_value);
                    break;
                case UCI_COMBO:
                    snprintf(state->uciOptions[i].value.combo, sizeof(state->uciOptions[i].value.combo), "%s", option_value);
                    break;
                case UCI_STRING:
                    snprintf(state->uciOptions[i].value.string, sizeof(state->uciOptions[i].value.string), "%s", option_value);
                    break;
                default:
                    printf("info string Unknown option type\n");
                    return;
            }
            printf("info string Option %s set to %s\n", option_name, option_value);
            return;
        }
    }

    printf("info string Unknown option: %s\n", option_name);
}

void go(State* state, const char* command)
{
    if (strncmp(command, "go perft ", 9) == 0) {
        int depth = atoi(command + 9);
        printf("depth: %d\n", depth);
        int nodes = Perft(&state->board, depth, true);
        printf("\nNodes searched: %d\n", nodes);
    } else {
        char bestmove[16];
        int score = 0;
        Move move = FindBest(&state->board, 1, &score);
        MoveToString(move, bestmove);
        printf("bestmove %s", bestmove);
#ifndef RELEASE
        printf(" (%d)", score);
#endif // RELEASE
        printf("\n");
    }
}

void position(State* state, const char* command)
{
    char fen[128] = "";

    if (strcmp(command, "position startpos") == 0) {
        strcpy(fen, STARTING_FEN);
    } else if (strncmp(command, "position fen ", 13) == 0) {
        strncpy(fen, command + 13, sizeof(fen) - 1);
        fen[sizeof(fen) - 1] = '\0'; // Ensure null-termination
    }

    if (strlen(fen) > 0) {
        StateSetStartPos(state, fen);
    } else {
        printf("info string Invalid position command\n");
    }

}

void StatePrint(const State* state)
{
    if (!state) {
        printf("State is NULL\n");
        return;
    }

    printf("Start Position FEN: %s\n", state->startPositionFen);
    printf("UCI Mode: %s\n", state->uciMode ? "Enabled" : "Disabled");
    printf("Debug Mode: %s\n", state->debugMode ? "Enabled" : "Disabled");
    printf("Depth Limit: %d\n", state->depthLimit);
    printf("Time Left - White: %d ms, Black: %d ms\n", state->timeLeft[0], state->timeLeft[1]);
    printf("Increment - White: %d ms, Black: %d ms\n", state->increment[0], state->increment[1]);
    printf("Moves to Go: %d\n", state->movesToGo);
    printf("Ponder Mode: %s\n", state->ponderMode ? "Enabled" : "Disabled");
    printf("Infinite Mode: %s\n", state->infiniteMode ? "Enabled" : "Disabled");
    printf("Stop Requested: %s\n", state->stopRequested ? "Yes" : "No");

    printf("\nUCI Options (%zu total):\n", state->uciOptionCount);
    for (size_t i = 0; i < state->uciOptionCount; i++) {
        const UciOption* opt = &state->uciOptions[i];
        printf("  [%zu] Name: %s\n", i + 1, opt->name);

        switch (opt->type) {
            case UCI_CHECK:
                printf("      Type: CHECK\n");
                printf("      Value: %s\n", opt->value.check ? "true" : "false");
                break;
            case UCI_SPIN:
                printf("      Type: SPIN\n");
                printf("      Value: %d (Min: %d, Max: %d)\n", opt->value.spin, opt->params.min, opt->params.max);
                break;
            case UCI_COMBO:
                printf("      Type: COMBO\n");
                printf("      Value: %s\n", opt->value.combo);
                printf("      Options: ");
                for (int j = 0; j < 10 && opt->params.combos[j][0] != '\0'; j++) {
                    printf("%s%s", opt->params.combos[j], (j < 9 && opt->params.combos[j + 1][0] != '\0') ? ", " : "");
                }
                printf("\n");
                break;
            case UCI_STRING:
                printf("      Type: STRING\n");
                printf("      Value: %s\n", opt->value.string);
                break;
        }
        printf("      Default: %s\n", opt->default_value);
    }

    printf("\nLast Command: %s\n", state->lastCommand);
}

void LoadUciConfig(State* state)
{
    LuaMan lua = {0};
    LMInit(&lua);

    LMPushNumber(&lua, UCI_CHECK,  "UCI_CHECK");
    LMPushNumber(&lua, UCI_SPIN,   "UCI_SPIN");
    LMPushNumber(&lua, UCI_COMBO,  "UCI_COMBO");
    LMPushNumber(&lua, UCI_STRING, "UCI_STRING");

    if (!LMRunFile(&lua, UCI_CONFIG_SCRIPT)) {
        fprintf(stderr, "Could not run %s (%s)\n", UCI_CONFIG_SCRIPT, lua_tostring(lua.state, -1));
        exit(1);
    }

    LMGetScriptReturn(&lua);
    if (!lua_istable(lua.state, -1)) {
        fprintf(stderr, "Expected a table from Lua, but got %s\n", luaL_typename(lua.state, -1));
        exit(1);
    }

    const char* fen = LMGetString(&lua, "startpos", VAR_LOCAL);
    printf("fen: %s\n", fen);
    if (fen != NULL && strcmp(fen, "") != 0) {
        snprintf(state->startPositionFen, sizeof(state->startPositionFen), "%s", fen);
    }

    state->uciMode      = LMGetBoolean(&lua, "uciMode", VAR_LOCAL);
    state->debugMode    = LMGetBoolean(&lua, "debugMode", VAR_LOCAL);
    state->depthLimit   = LMGetNumber (&lua, "depthLimit", VAR_LOCAL);
    state->ponderMode   = LMGetBoolean(&lua, "ponderMode", VAR_LOCAL);
    state->infiniteMode = LMGetBoolean(&lua, "infiniteMode", VAR_LOCAL);

if (LMGetTable(&lua, "options", VAR_LOCAL)) {
    lua_pushnil(lua.state);  // Push nil to start iterating the table
    while (lua_next(lua.state, -2) != 0) {
        // Ensure we do not go beyond the array bounds
        if (state->uciOptionCount >= MAX_UCI_OPTIONS) {
            fprintf(stderr, "Too many UCI options! Increase MAX_UCI_OPTIONS.\n");
            lua_pop(lua.state, 2);  // Pop the current key-value pair and the 'options' table
            break;
        }

        UciOption* opt = &state->uciOptions[state->uciOptionCount];

        // Accessing the 'name' field safely
        const char* name = LMGetString(&lua, "name", VAR_LOCAL);
        if (name) {
            strncpy(opt->name, name, sizeof(opt->name) - 1);
            opt->name[sizeof(opt->name) - 1] = '\0';
        } else {
            fprintf(stderr, "Skipping option with missing 'name'\n");
            lua_pop(lua.state, 1);  // Pop the current value and continue
            continue;
        }

        // Accessing 'type' field and processing options accordingly
        opt->type = (int)LMGetNumber(&lua, "type", VAR_LOCAL);
        switch (opt->type) {
            case UCI_SPIN:
                opt->value.spin = LMGetNumber(&lua, "value", VAR_LOCAL);
                opt->params.min = LMGetTableFieldNumber(&lua, "params", "min", VAR_LOCAL);
                opt->params.max = LMGetTableFieldNumber(&lua, "params", "max", VAR_LOCAL);
                break;
            case UCI_CHECK:
                opt->value.check = LMGetBoolean(&lua, "value", VAR_LOCAL);
                break;
            case UCI_COMBO:
                strncpy(opt->value.combo, LMGetString(&lua, "value", VAR_LOCAL), sizeof(opt->value.combo) - 1);
                opt->value.combo[sizeof(opt->value.combo) - 1] = '\0';

                // Handling combo options (params as a table)
                lua_getfield(lua.state, -1, "params");
                if (lua_istable(lua.state, -1)) {
                    lua_pushnil(lua.state);  // Push nil to iterate over params
                    int count = 0;
                    while (count < 10 && lua_next(lua.state, -2) != 0) {
                        // Ensure we're copying combo options correctly
                        const char* combo_val = lua_tostring(lua.state, -1);
                        if (combo_val) {
                            strncpy(opt->params.combos[count], combo_val, sizeof(opt->params.combos[count]) - 1);
                            opt->params.combos[count][sizeof(opt->params.combos[count]) - 1] = '\0';
                            count++;
                        }
                        lua_pop(lua.state, 1);  // Pop the value after processing
                    }
                }
                lua_pop(lua.state, 1);  // Pop 'params' table
                break;
            case UCI_STRING:
                strncpy(opt->value.string, LMGetString(&lua, "value", VAR_LOCAL), sizeof(opt->value.string) - 1);
                opt->value.string[sizeof(opt->value.string) - 1] = '\0';
                break;
            default:
                fprintf(stderr, "Incorrect type value: %d\n", opt->type);
                lua_pop(lua.state, 1);  // Pop the current value and exit
                exit(1);
        }

        const char* default_value = LMGetString(&lua, "default", VAR_LOCAL);
        if (default_value) {
            strncpy(opt->default_value, default_value, sizeof(opt->default_value) - 1);
            opt->default_value[sizeof(opt->default_value) - 1] = '\0';
        }

        state->uciOptionCount++;

        lua_pop(lua.state, 1);  // Pop the value after processing each pair
    }
}
    
    LMClose(&lua);
}
