#include "uci.h"

#define LUAMAN_IMPLEMENTATION
#include "extern/luaman.h"

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
        LMClose(&lua);
        exit(1);
    }

    LMGetScriptReturn(&lua);
    if (!lua_istable(lua.state, -1)) {
        fprintf(stderr, "Expected a table from Lua, but got %s\n", luaL_typename(lua.state, -1));
        LMClose(&lua);
        exit(1);
    }

    const char* fen = LMGetString(&lua, "startpos", VAR_LOCAL);
    if (fen != NULL && strcmp(fen, "") != 0) {
        snprintf(state->startPositionFen, sizeof(state->startPositionFen), "%s", fen);
    }

    state->uciMode      = LMGetBoolean(&lua, "uciMode", VAR_LOCAL);
    state->debugMode    = LMGetBoolean(&lua, "debugMode", VAR_LOCAL);
    state->depthLimit   = LMGetNumber (&lua, "depthLimit", VAR_LOCAL);
    state->ponderMode   = LMGetBoolean(&lua, "ponderMode", VAR_LOCAL);
    state->infiniteMode = LMGetBoolean(&lua, "infiniteMode", VAR_LOCAL);
    state->maxBookmoves = LMGetNumber (&lua, "maxBookmoves", VAR_LOCAL);

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
                    LMClose(&lua);
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

