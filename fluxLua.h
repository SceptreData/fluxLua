/*
 * fluxLua
 * 0.0.1
 * Bindings to interact and load lua scripts from C.
 */

#ifndef FLUXLUA_H
#define FLUXLUA_H


#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <string.h>

typedef struct LuaScript_s
{
    lua_State *L;
    char *filename;
} LuaScript_t;

typedef struct LuaVar_s{
    char *name;
    char *str;
    float val;
} LuaVar_t;

#endif
