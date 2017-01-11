#ifndef FLUXLUA_UTIL_H
#define FLUXLUA_UTIL_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <string.h>

static void flux_ClearLuaStack (lua_State *L);
static char *flux_strdup (const char *str);

#endif
