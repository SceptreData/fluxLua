#include "fluxLua_Script.h"

/* Utility Function Forward Declarations */
static char *flux_strdup(const char *str);
static void flux_ClearLuaStack(lua_State *L);

// -------------------------------------------------------- //
/* LuaVar_t Functions */

static LuaVar_t *LuaVar_New(char *varName)
{
    LuaVar_t *var = malloc(sizeof(*var));
    if (!var) {
        MemoryError();
    }
    var->name = strdup(varName);
    var->str = NULL;
    var->val = 0;

    return var;
}

void LuaVar_Set(LuaVar_t *var, const char *value)
{
    var->str = flux_strdup(value);
    var->val = atof(value);
}

/* LuaVar_Parse()
 * Tries to parse the Lua object currently at the top of the stack
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
    // lua_tostring automatically converts our Lua numbers into strings.
    if (lua_isstring(L, -1) || lua_isnumber(L, -1)){
         LuaVar_Set(var, lua_tostring(L, -1));
    }
    else if (lua_isboolean(L, -1)){
        if (lua_toboolean(L, -1) == 1){
            LuaVar_Set(var, "1");
        } else {
            LuaVar_Set(var, "0");
        }
    } else {
        printf("ERROR: Trying to parse invalid variable type.(Is it a table?)\n");
        return 1;
    }
    return 0;
}

// Build an array of LuaVar_t values out of the table on the top of the Lua stack.
static LuaVar_t *LuaVar_BuildTable(lua_State *L, char *tableName)
{
    if (L == NULL || !lua_istable(L, -1)) {
        printf("ERROR: Unable to load table %s. Did you pass a table?\n", tableName);
        return NULL;
    }

    size_t tableLen = lua_rawlen(L, -1);
    LuaVar_t *table = malloc(sizeof(LuaVar_t) * (tableLen + 1));
    if (!table) {
        MemoryError();
    }

    // Use our first value in the array to hold the tableName and the size of our array.
    table[0].name = strdup(tableName);
    table[0].str = strdup("TABLE");
    table[0].val = tableLen;


    int idx = 1; 
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        LuaVar_Parse(L, &table[idx]);
        lua_pop(L, 1);
        idx++;
    }
    return table;
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

void LuaVar_FreeTable( LuaVar_t *table )
{
    if (table){
        int tableLen = (int)table[0].val + 1;
        LuaVar_t *p = table;
        for (int i = 0; i < tableLen; i++) {
            if (p->name)
                free(p->name);
            if (p->str)
                free(p->str);
            p++;
        }
        free(table);
    }
}

// ----------------------------------------------------------------
/* LuaScript_t Functions */

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

    // Load our .lua file, report any errors.
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

/* LuaScript_FindVar()
 * This function splits the provided variable name and navigates through the Lua
 * tables to find the associated variable value.
 * If we find the value, we push it onto the Lua stack so other functions can access it.
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
            printf("ERROR: Unable to find object '%s' in %s from %s.\n",
                    curVar, varName, script->filename);
            return 0;
        } else {
            curVar = strtok(NULL, ".");
            level += 1;
        }
    }
    return 1;
}

LuaVar_t *LuaScript_GetVar (LuaScript_t *script, char *varName)
{
    if (script == NULL || script->L == NULL) {
        printf("ERROR: Tried to Get variable from un-loaded script '%s'.\n", script->filename);
        return NULL;
    }

    LuaVar_t *var = NULL;
    // Try to find our variable in the LuaScript, then push it to the top of the Lua stack.
    // If we find it, parse the value into an LuaVar_t
    if (LuaScript_FindVar(script, varName)) {
        var = LuaVar_New(varName);
        // if LuaVar_Parse returns an error, free our poor LuaVar
        if (LuaVar_Parse(script->L, var)){
            LuaVar_Free(var);
            var = NULL;
        }
    } else {
        printf("ERROR: Could not find '%s' in loaded script '%s'\n", varName, script->filename);
        return NULL;
    }
    flux_ClearLuaStack(script->L);
    return var;
}


LuaVar_t *LuaScript_GetTable (LuaScript_t *script, char *tableName)
{
    if (script == NULL || script->L == NULL) {
        printf("ERROR: Tried to Get variable from un-loaded script '%s'.\n", script->filename);
        return NULL;
    }

    LuaVar_t *table = NULL;
    if (LuaScript_FindVar(script, tableName)) {
        table = LuaScript_BuildTable(script->L, tableName);
    }
    flux_ClearLuaStack(script->L);

    return table;
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
        script = NULL;
    }
}

// ----------------------------------------------------------------//
/* Utility  Functions */
static char *flux_strdup(const char *str)
{
    char *p = malloc(strlen(str) + 1);
    if (!p){
        MemoryError();
    }
    return strcpy(p, str);
}

static void flux_ClearLuaStack(lua_State *L)
{
    int n = lua_gettop(L);
    lua_pop(L, n);
}
