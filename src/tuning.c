#include "tuning.h"
#include "extern/luaman.h"
#include <limits.h>

void PrintTuning(const Tuning* tuning)
{
    printf("Pawn value: %d\n", tuning->pieces.pawn);
    printf("Knight value: %d\n", tuning->pieces.knight);
    printf("Bishop value: %d\n", tuning->pieces.bishop);
    printf("Rook value: %d\n", tuning->pieces.rook);
    printf("Queen value: %d\n", tuning->pieces.queen);
    printf("King value: %d\n", tuning->pieces.king);
}

void LoadTuning(Tuning* tuning)
{
    LuaMan lua = {0};
    LMInit(&lua);

    LMPushNumber(&lua, INT_MAX, "inf");

    if (!LMRunFile(&lua, TUNING_SCRIPT)) {
        fprintf(stderr, "Could not run %s (%s)\n", TUNING_SCRIPT, lua_tostring(lua.state, -1));
        exit(1);
    }

    LMGetScriptReturn(&lua);
    if (!lua_istable(lua.state, -1)) {
        fprintf(stderr, "Expected a table from Lua, but got %s\n", luaL_typename(lua.state, -1));
        exit(1);
    }
    
    const char* pieces = "pieces";
    tuning->pieces.pawn = LMGetTableFieldNumber(&lua, pieces, "pawn", VAR_LOCAL);
    tuning->pieces.knight = LMGetTableFieldNumber(&lua, pieces, "knight", VAR_LOCAL);
    tuning->pieces.bishop = LMGetTableFieldNumber(&lua, pieces, "bishop", VAR_LOCAL);
    tuning->pieces.rook = LMGetTableFieldNumber(&lua, pieces, "rook", VAR_LOCAL);
    tuning->pieces.queen = LMGetTableFieldNumber(&lua, pieces, "queen", VAR_LOCAL);
    tuning->pieces.king = LMGetTableFieldNumber(&lua, pieces, "king", VAR_LOCAL);

    LMClose(&lua);
}

