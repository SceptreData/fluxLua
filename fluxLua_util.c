#include "fluxLua_util.h"

static void flux_ClearLuaStack(lua_State *L)
{
    int n = lua_gettop(L);
    lua_pop(L, n);
}

static char *flux_strdup(const char *str)
{
    char *p = malloc(strlen(str) + 1);
    if (!p){
        MemoryError();
    }
    return strcpy(p, str);
}
