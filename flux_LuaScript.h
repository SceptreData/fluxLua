/*
 * fluxLua
 *     David Bergeron (2016)
 *     0.1.0
 *     Bindings to load lua scripts and values into C programs.
 */

#ifndef FLUXLUA_SCRIPT_H
#define FLUXLUA_SCRIPT_H


#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <stdlib.h>
#include <string.h>

#define MemoryError() do { printf("MEMORY ERROR: Line %d\n", __LINE__); exit(-1); } while(0)

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

/*
================
LuaScript_Load()

Loads a .lua Script into a new Lua state so we can access it's values.
Returns a new LuaScript_t object.
================
*/
LuaScript_t *LuaScript_Load (char *filename);

/*
================
LuaScript_GetVar()

Tries to retrieve a variable from a loaded Lua script.
Returns an LuaVar_t, containing both a float and a string
representation of the value. Throws an error if the object pointed to
is a table.
================
*/
LuaVar_t *LuaScript_GetVar (LuaScript_t *script, char *varName);

/*
================
LuaScript_GetTable()

Tries to retrieve a table from a loaded Lua script.
Returns an array of LuaVar_t if succesful.
This array contains (N + 1) elements, where N is the length of the table in lua.
The first element of the array is used to provide information about the array.
The str field contains the string "_TABLE" to identify the LuaVar_t as a table,
the val field contains the length of our array to facilitate iteration.
This means that if you want to iterate starting at the first real element of the table,
start by accessing the element at index 1. (The second element in the array).

EX: int num_elts = table[0].val;
    for (int i = 1; i <= num_elts; i++){
        ...
    }
================
*/
LuaVar_t *LuaScript_GetTable (LuaScript_t *script, char *tableName);

/*
================
LuaVar_Set()

Allows you to modify an already allocated LuaVar_t. Does not
modify the original script value.
================
*/
void LuaVar_Set(LuaVar_t *var, const char *value);

/*
================
Destructor Functions
================
*/
void LuaScript_Free( LuaScript_t *script );
void LuaVar_Free( LuaVar_t *var );
void LuaVar_FreeTable( LuaVar_t *table );
#endif
