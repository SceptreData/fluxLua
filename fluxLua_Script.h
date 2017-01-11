/*
 * fluxLua
 * 0.0.1
 * Bindings to interact and load lua scripts from C.
 */

#ifndef FLUXLUA_SCRIPT_H
#define FLUXLUA_SCRIPT_H


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

LuaScript_t *LuaScript_Load (char *filename);

LuaVar_t *LuaScript_GetVar (LuaScript_t *script, char *varName);

LuaVar_t *LuaScript_GetTable (LuaScript_t *script, char *tableName);

void LuaVar_Set(LuaVar_t *var, const char *value);

void LuaScript_Free( LuaScript_t *script );

void LuaVar_Free( LuaVar_t *var );

void LuaVar_FreeTable( LuaVar_t *table );
#endif
