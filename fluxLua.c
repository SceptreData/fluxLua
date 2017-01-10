#include "fluxLua.h"

#define MemoryError() do{printf("ERROR: Ran out of memory"); exit(-1);}while(0);

static LuaScript_t *lua_NewScript(void);
static int lua_GetVarToStack( LuaScript_t *script, const char *varName);
static void lua_CleanStack(lua_State *L);

LuaVar_t *flux_NewLuaVar(void);

static char *flux_strdup (const char *str);

LuaScript_t *lua_LoadScript( char *filename )
{
    if (filename == NULL) {
        printf("ERROR: Invalid filename passed to loader");
        return NULL;
    }

    // Generate a new Lua state for our script.
    lua_State *L = luaL_newstate();

    /* Try to load the Lua file, then activate it.
       Both of these functions return a non-zero value on fail */
    int err = (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0));
    if (err) {
        printf("ERROR: Failed to load script %s\n", filename);
        return NULL;
    }

    // Now that we've succesfully loaded our script, allocate our LuaScript_t struct.
    LuaScript_t *script = lua_NewScript();
    script->filename = flux_strdup(filename);
    script->L = L;

    return script;
}

void flux_SetLuaVar(LuaVar_t *var, const char *value)
{
    var->str = flux_strdup(value);
    var->val = atof(value);
}

int lua_ParseLuaVar(lua_State *L, LuaVar_t *var)
{
    if (!L) {
       printf("ERROR: Trying to parse Variable from inactive Lua state.\n"); 
       return 1;
    }

    if (lua_isnil(L, -1)) {
        printf("ERROR: Unable to parse LuaVar, Lua stack value is nil.\n");
        return 1;
    }

    if (lua_isstring(L, -1) || lua_isnumber(L, -1)){
         flux_SetLuaVar(var, lua_tostring(L, -1));
    }
    else if (lua_isboolean(L, -1)){
        if (lua_toboolean(L, -1) == 1){
            flux_SetLuaVar(var, "1");
        } else {
            flux_SetLuaVar(var, "0");
        }
    } else {
        printf("ERROR: Trying to parse invalid variable type.\n");
        return 1;
    }
    return 0;
}

int lua_GetScriptVar (LuaScript_t *script, LuaVar_t *var, char *varName)
{
    if (script->L == NULL) {
        printf("ERROR: Tried to Get variable from un-loaded script '%s'.\n", script->filename);
        return 1;
    }
    var->name = strdup(varName);
    // Try to find our variable name in the script.
    // If we find it, parse the value into an LuaVar_t
    if (lua_GetVarToStack(script, varName)) {
        lua_ParseLuaVar(script->L, var);
    } else {
        printf("ERROR: Could not find '%s' in loaded script '%s'\n", varName, script->filename);
        return 1;
    }
    lua_CleanStack(script->L);
    return 0;
}

static int lua_GetVarToStack( LuaScript_t *script, const char *varName)
{
    lua_State *L = script->L;
    // Duplicate our variable name string, so strtok doesn't affect the original.
    char *curVar = flux_strdup(varName);

    // Split the string at every '.' separator.
    curVar = strtok(curVar, ".");

    // Iterate over the now separated portions of our string.
    int level = 0;
    while (curVar != NULL) {

        // If we are at the top level of the variable, it will be a global identifier.
        if (level == 0) {
            lua_getglobal(L, curVar);
        // Otherwise, we are accessing a value inside a table, push the field to the stack.
        } else {
            lua_getfield(L, -1, curVar);
        }

        if (lua_isnil(L, -1)) {
            printf("ERROR: Unable to find variable '%s' in %s from %s.\n",
                    curVar, varName, script->filename);
            return 1;
        } else {
            level += 1;
        }
    }
    return 0;
}

static LuaScript_t *lua_NewScript(void)
{
    LuaScript_t *new = malloc(sizeof(*new));
    if (new == NULL){
        MemoryError();
    }
    new->L = NULL;
    new->filename = NULL;

    return new;
}

static void lua_CleanStack(lua_State *L)
{
    int n = lua_gettop(L);
    lua_pop(L, n);
}

LuaVar_t *flux_NewLuaVar(void)
{
    LuaVar_t *var = malloc(sizeof(*var));
    if (!var) {
        MemoryError();
    }
    var->name = NULL;
    var->str = NULL;
    var->val = 0;

    return var;
}

static char *flux_strdup(const char *str)
{
    char *p = malloc(strlen(str) + 1);
    if (!p){
        MemoryError();
    }
    return strcpy(p, str);
}
