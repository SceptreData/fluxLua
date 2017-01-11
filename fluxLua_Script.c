#include "flux_LuaScript.h"

#define MemoryError() do{printf("ERROR: Ran out of memory"); exit(-1);}while(0);

static LuaScript_t *LuaScript_New(void)
{
    LuaScript_t *new = malloc(sizeof(*new));
    if (new == NULL){
        MemoryError();
    }
    new->L = NULL;

    return new;
}


LuaScript_t *LuaScript_Load( char *filename )
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
    LuaScript_t *script = LuaScript_New();
    script->filename = strdup(filename);
    script->L = L;

    return script;
}

/* LuaVar_t initializer */
LuaVar_t *LuaVar_New(void)
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

void LuaVar_Set(LuaVar_t *var, const char *value)
{
    var->str = flux_strdup(value);
    var->val = atof(value);
}

// Forward declarations.
static int LuaScript_FindVar ( LuaScript_t *script, const char *varName);
static int LuaScript_ParseVar (lua_State *L, LuaVar_t *var);

LuaVar_t *LuaScript_GetVar (LuaScript_t *script, char *varName)
{
    if (script->L == NULL) {
        printf("ERROR: Tried to Get variable from un-loaded script '%s'.\n", script->filename);
        return 1;
    }

    LuaVar_t *var = LuaVar_New();

    // Try to find our variable in the LuaScript, then push it to the top of the Lua stack.
    if (LuaScript_FindVar(script, varName)) {
        // If we find it, parse the value into an LuaVar_t
        LuaVar_Parse(script->L, var);
    } else {
        printf("ERROR: Could not find '%s' in loaded script '%s'\n", varName, script->filename);
        return 1;
    }
    flux_ClearLuaStack(script->L);
    return 0;
}

/* LuaScript_FindVar()
 * This function splits the provided variable name and navigates through the Lua
 * tables to find the associated variable value.
 * If we find the value, we push it onto the Lua stack so other functions can acces it.
 */
static int LuaScript_FindVar( LuaScript_t *script, const char *varName)
{
    // we use strtok() to split our variable name on the periods, so we have to duplicate
    // the string first so we don't modify the original.
    char *curVar = flux_strdup(varName);
    curVar = strtok(curVar, ".");

    // Iterate over the now separated portions of our string.
    int level = 0;
    while (curVar != NULL) {

        if (level == 0) { // If we are at the top level of the variable, it will be a global identifier.
            lua_getglobal(script->L, curVar);
        } else {
            lua_getfield(script->L, -1, curVar); // Otherwise, value is inside a table, so we use lua_getfield.
        }

        if (lua_isnil(script->L, -1)) {
            printf("ERROR: Unable to find variable '%s' in %s from %s.\n",
                    curVar, varName, script->filename);
            return 1;
        } else {
            level += 1;
        }
    }
    return 0;
}

/* LuaVar_Parse()
 * This function tries to parse the Lua object currently at the top of the stack
 * into an LuaVar_t.
 * The Lua object at the top of the stack can be any string, value or boolean,
 * but it will always be stored as both a float AND a string.
 */
static int LuaVar_Parse(lua_State *L, LuaVar_t *var)
{
    if (!L) {
       printf("ERROR: Trying to parse Variable from inactive Lua state.\n"); 
       return 1;
    }

    if (lua_isnil(L, -1)) {
        printf("ERROR: Unable to parse LuaVar, top of stack is nil.\n");
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

void LuaScript_Free( LuaScript_t *script )
{
    if (script) {
        if (script->L) {
            lua_close(script->L);
        }
        if (script->filename){
            free(script->filename);
        }
        free(script);
    }
}

void LuaVar_Free( LuaVar_t *var )
{
    if (var) {
        if (var->name) {
            free(var->name);
        }
        if (var->str) {
            free(var->str);
        }
        free(var);
    }
}
