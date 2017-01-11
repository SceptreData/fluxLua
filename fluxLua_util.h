#ifndef FLUXLUA_UTIL_H
#define FLUXLUA_UTIL_H
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdlib.h>
#include <string.h>

#define MemoryError() do{printf("ERROR: Ran out of memory"); exit(-1);}while(0)

void flux_ClearLuaStack (lua_State *L);
char *flux_strdup (const char *str);

#endif
