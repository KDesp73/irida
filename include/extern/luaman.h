#ifndef LUAMAN_H
#define LUAMAN_H

#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define LM_VERSION_MAJOR 0
#define LM_VERSION_MINOR 1
#define LM_VERSION_PATCH 0
#define LM_VERSION "0.1.0"

#ifndef LMAPI
    #define LMAPI extern
#endif // LMAPI

typedef struct {
    lua_State* state;
} LuaMan;

enum {
    VAR_LOCAL,
    VAR_GLOBAL
};

// Initialization
LMAPI void LMInit(LuaMan* lm);
LMAPI void LMClose(LuaMan* lm);

// Pushing values
LMAPI void LMPushFunction(LuaMan* lm, lua_CFunction fun, const char* name);
LMAPI void LMPushNumber(LuaMan* lm, double number, const char* name);
LMAPI void LMPushString(LuaMan* lm, const char* string, const char* name);
LMAPI void LMPushBoolean(LuaMan* lm, bool boolean, const char* name);

// Retrieving values
LMAPI double LMGetNumber(LuaMan* lm, const char* name, int scope);
LMAPI char* LMGetString(LuaMan* lm, const char* name, int scope);
LMAPI bool LMGetBoolean(LuaMan* lm, const char* name, int scope);

// Table handling
LMAPI bool LMGetTable(LuaMan* lm, const char* name, int scope);
LMAPI double LMGetTableFieldNumber(LuaMan* lm, const char* table, const char* field, int scope);
LMAPI char* LMGetTableFieldString(LuaMan* lm, const char* table, const char* field, int scope);
LMAPI void LMGetScriptReturn(LuaMan* lm);

// Function calls
LMAPI bool LMGetFunction(LuaMan* lm, const char* name);
LMAPI bool LMCallFunction(LuaMan* lm, int args, int returns);

#define LMCallFunctionBegin(lm, func, arg_count, ret_count) \
        lua_getglobal((lm)->state, func); \
        if(lua_isfunction((lm)->state, -1)) { \
            const int __arguments_count__ = arg_count; \
            const int __returnvalues_count__ = ret_count; \

#define LMCallFunctionEnd(lm) \
            lua_pcall((lm)->state, __arguments_count__, __returnvalues_count__, 0); \
        }

#define LMParamNumber(lm, num) LMPushNumber(lm, num, NULL, VAR_LOCAL);
#define LMParamString(lm, str) LMPushString(lm, str, NULL, VAR_LOCAL);

// Running scripts
#define LMRunString(lm, string) (luaL_dostring((lm)->state, string) == LUA_OK)
#define LMRunFile(lm, path) (luaL_dofile((lm)->state, path) == LUA_OK)

#endif // LUAMAN_H

#ifdef LUAMAN_IMPLEMENTATION

// --------------------- Initialization ---------------------

LMAPI void LMInit(LuaMan* lm)
{
    assert(lm);
    lm->state = luaL_newstate();
    luaL_openlibs(lm->state);
}

LMAPI void LMClose(LuaMan* lm)
{
    if (lm->state) {
        lua_close(lm->state);
        lm->state = NULL;
    }
}

// --------------------- Pushing Values ---------------------

LMAPI void LMPushNumber(LuaMan* lm, double number, const char* name)
{
    assert(lm);
    lua_pushnumber(lm->state, number);
    if (name) {
        lua_setglobal(lm->state, name);
    }
}

LMAPI void LMPushString(LuaMan* lm, const char* string, const char* name)
{
    assert(lm);
    assert(string);
    lua_pushstring(lm->state, string);
    if (name) {
        lua_setglobal(lm->state, name);
    }
}

LMAPI void LMPushBoolean(LuaMan* lm, bool boolean, const char* name)
{
    assert(lm);
    assert(boolean);
    lua_pushboolean(lm->state, boolean);
    if (name) {
        lua_setglobal(lm->state, name);
    }
}

LMAPI void LMPushFunction(LuaMan* lm, lua_CFunction fun, const char* name)
{
    assert(lm && fun && name);
    lua_pushcfunction(lm->state, fun);
    lua_setglobal(lm->state, name);
}

// --------------------- Retrieving Values ---------------------

LMAPI double LMGetNumber(LuaMan* lm, const char* name, int scope)
{
    assert(lm && name);
    switch (scope) {
        case VAR_LOCAL:
            lua_getfield(lm->state, -1, name);
            break;
        case VAR_GLOBAL:
            lua_getglobal(lm->state, name);
            break;
    }
    double result = lua_tonumber(lm->state, -1);
    lua_pop(lm->state, 1);
    return result;
}

LMAPI char* LMGetString(LuaMan* lm, const char* name, int scope)
{
    assert(lm && name);
    switch (scope) {
        case VAR_LOCAL:
            lua_getfield(lm->state, -1, name);
            break;
        case VAR_GLOBAL:
            lua_getglobal(lm->state, name);
            break;
    }
    const char* str = lua_tostring(lm->state, -1);
    char* copy = str ? strdup(str) : NULL;
    lua_pop(lm->state, 1);
    return copy;
}

LMAPI bool LMGetBoolean(LuaMan* lm, const char* name, int scope)
{
    assert(lm && name);
    switch (scope) {
        case VAR_LOCAL:
            lua_getfield(lm->state, -1, name);
            break;
        case VAR_GLOBAL:
            lua_getglobal(lm->state, name);
            break;
    }
    bool result = lua_toboolean(lm->state, -1);
    lua_pop(lm->state, 1);
    return result;
}

// --------------------- Table Handling ---------------------

LMAPI bool LMGetTable(LuaMan* lm, const char* name, int scope)
{
    assert(lm && name);
    switch (scope) {
        case VAR_LOCAL:
            lua_getfield(lm->state, -1, name);
            break;
        case VAR_GLOBAL:
            lua_getglobal(lm->state, name);
            break;
    }
    return lua_istable(lm->state, -1);
}

LMAPI double LMGetTableFieldNumber(LuaMan* lm, const char* table, const char* field, int scope)
{
    assert(lm && table && field);
    if (!LMGetTable(lm, table, scope)) {
        return 0.0;
    }
    lua_getfield(lm->state, -1, field);
    double result = lua_tonumber(lm->state, -1);
    lua_pop(lm->state, 2);
    return result;
}

LMAPI char* LMGetTableFieldString(LuaMan* lm, const char* table, const char* field, int scope)
{
    assert(lm && table && field);
    if (!LMGetTable(lm, table, scope)) {
        return NULL;
    }
    lua_getfield(lm->state, -1, field);
    const char* str = lua_tostring(lm->state, -1);
    char* copy = str ? strdup(str) : NULL;
    lua_pop(lm->state, 2);
    return copy;
}

LMAPI void LMGetScriptReturn(LuaMan* lm)
{
    assert(lm);
    luaL_checktype(lm->state, -1, LUA_TTABLE);
}

// --------------------- Function Handling ---------------------

LMAPI bool LMGetFunction(LuaMan* lm, const char* name)
{
    assert(lm && name);
    lua_getglobal(lm->state, name);
    if (!lua_isfunction(lm->state, -1)) {
        lua_pop(lm->state, 1);
        return false;
    }
    return true;
}

LMAPI bool LMCallFunction(LuaMan* lm, int args, int returns)
{
    assert(lm);
    if (lua_pcall(lm->state, args, returns, 0) != LUA_OK) {
        const char* error_msg = lua_tostring(lm->state, -1);
        fprintf(stderr, "Lua Error: %s\n", error_msg);
        lua_pop(lm->state, 1);
        return false;
    }
    return true;
}

#endif // LUAMAN_IMPLEMENTATION
