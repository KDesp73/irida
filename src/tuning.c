#include "tuning.h"
#include "extern/luaman.h"
#include "lua.h"
#include <limits.h>
#include <stdio.h>

void PrintTuning(const Tuning* tuning)
{
    printf("Pawn value: %d\n", tuning->pieces.pawn);
    printf("Knight value: %d\n", tuning->pieces.knight);
    printf("Bishop value: %d\n", tuning->pieces.bishop);
    printf("Rook value: %d\n", tuning->pieces.rook);
    printf("Queen value: %d\n", tuning->pieces.queen);
    printf("King value: %d\n", tuning->pieces.king);
    printf("\n");
    printf("Middlegame moves: %zu\n", tuning->thresholds.middlegameMoves);
    printf("Middlegame pieces: %zu\n", tuning->thresholds.middlegamePieces);
    printf("Endgame moves: %zu\n", tuning->thresholds.endgameMoves);
    printf("Endgame piece: %zu\n", tuning->thresholds.endgamePieces);
    printf("\n");
}

void LoadTuning(Tuning* tuning)
{
    LuaMan lua = {0};
    LMInit(&lua);

    LMPushNumber(&lua, INT_MAX, "inf");

    if (!LMRunFile(&lua, TUNING_SCRIPT)) {
        fprintf(stderr, "Could not run %s (%s)\n", TUNING_SCRIPT, lua_tostring(lua.state, -1));
        LMClose(&lua);
        exit(1);
    }

    LMGetScriptReturn(&lua);
    if (!lua_istable(lua.state, -1)) {
        fprintf(stderr, "Expected a table from Lua, but got %s\n", luaL_typename(lua.state, -1));
        LMClose(&lua);
        exit(1);
    }
    
    // Load piece values
    const char* pieces = "pieces";
    tuning->pieces.pawn   = LMGetTableFieldNumber(&lua, pieces, "pawn", VAR_LOCAL);
    tuning->pieces.knight = LMGetTableFieldNumber(&lua, pieces, "knight", VAR_LOCAL);
    tuning->pieces.bishop = LMGetTableFieldNumber(&lua, pieces, "bishop", VAR_LOCAL);
    tuning->pieces.rook   = LMGetTableFieldNumber(&lua, pieces, "rook", VAR_LOCAL);
    tuning->pieces.queen  = LMGetTableFieldNumber(&lua, pieces, "queen", VAR_LOCAL);
    tuning->pieces.king   = LMGetTableFieldNumber(&lua, pieces, "king", VAR_LOCAL);

    // Load thresholds (nested tables)
    const char* thresholds = "thresholds";
    lua_getfield(lua.state, -1, thresholds);  // Push 'thresholds' table onto the stack
    if (!lua_istable(lua.state, -1)) {
        fprintf(stderr, "Expected a table for '%s', but got %s\n", thresholds, luaL_typename(lua.state, -1));
        exit(1);
    }

    // Load middlegame values
    tuning->thresholds.middlegameMoves  = LMGetTableFieldNumber(&lua, "middlegame", "moves", VAR_LOCAL);
    tuning->thresholds.middlegamePieces = LMGetTableFieldNumber(&lua, "middlegame", "pieces", VAR_LOCAL);

    // Load endgame values
    tuning->thresholds.endgameMoves  = LMGetTableFieldNumber(&lua, "endgame", "moves", VAR_LOCAL);
    tuning->thresholds.endgamePieces = LMGetTableFieldNumber(&lua, "endgame", "pieces", VAR_LOCAL);

    if(tuning->thresholds.middlegamePieces <= tuning->thresholds.endgamePieces){
        fprintf(stderr, "Threshold: Middlegame pieces (%zu) must be greater than endgame pieces (%zu)\n", tuning->thresholds.middlegamePieces, tuning->thresholds.endgamePieces);

        lua_pop(lua.state, 1);  // Pop 'thresholds' table
        LMClose(&lua);

        fprintf(stderr, "Aborting...\n");
        exit(1);
    }

    if(tuning->thresholds.middlegameMoves >= tuning->thresholds.endgameMoves){
        fprintf(stderr, "Threshold: Middlegame moves (%zu) must be less than endgame moves (%zu)\n", tuning->thresholds.middlegameMoves, tuning->thresholds.endgameMoves);

        lua_pop(lua.state, 1);  // Pop 'thresholds' table
        LMClose(&lua);

        fprintf(stderr, "Aborting...\n");
        exit(1);
    }

    lua_pop(lua.state, 1);  // Pop 'thresholds' table

    LMClose(&lua);
}
