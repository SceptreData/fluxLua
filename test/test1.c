#include "../flux_LuaScript.h"
#include <assert.h>

int Pass(int testNum)
{
    printf("\n---------------------------------------\n");
    printf("    TEST %d COMPLETE\n", testNum);
}

void TestOne(LuaScript_t *script)
{
    LuaVar_t *g_number = LuaScript_Get(script, "global_number");
    assert(strcmp(g_number->name, "global_number") == 0);
    assert(g_number->val == 10.0);

    Pass(1);
}

void TestTwo(LuaScript_t *script)
{
    LuaVar_t *g_string = LuaScript_Get(script, "global_string");
    LuaVar_t *g_true = LuaScript_Get(script, "global_true_bool");
    LuaVar_t *g_false = LuaScript_Get(script, "global_false_bool");

    assert(strcmp(g_string->str, "Test String!") == 0);
    assert(g_true->val == 1.0);
    assert(g_false->val == 0.0);

    Pass(2);
}
void TestThree(LuaScript_t *script)
{
    LuaVar_t *nest_num = LuaScript_Get(script, "nested_table.number");
    LuaVar_t *nest_str = LuaScript_Get(script, "nested_table.string");
    LuaVar_t *nest_bool= LuaScript_Get(script, "nested_table.bool");

    assert(nest_num->val == 666.0);
    assert(!strcmp(nest_str->str, "Loaded the nested String!"));
    assert(nest_bool->val == 1.0);

    Pass(3);
}

void TestFour(LuaScript_t *script)
{
    LuaVar_t *t = LuaScript_Get(script, "nested_table");
    printf("Table name: %s\nTable string: %s\nTable Val: %d\n", t->name, t->str, t->val);
    assert(!strcmp(t->str, "_TABLE"));
    Pass(4);
}

void TestFive(LuaScript_t *script)
{
    LuaVar_t *arr = LuaScript_Get(script, "global_array");
    assert(arr[0].val == 4.0);
    printf("Array Values: \n");
    for (int i = 1; i < 5; i++) {
        assert(arr[i].val);
        printf("%.1f ", arr[i].val); 
    }

    Pass(5);
}

void TestSix(LuaScript_t *script)
{
    LuaVar_t *n_arr = LuaScript_Get(script, "nested_array.nums");
    assert(n_arr[0].val == 4.0);
    for (int i = 1; i < 5; i++) {
        assert(n_arr[i].val);
        printf("%.1f ", n_arr[i].val); 
    }
    printf("\n");

    LuaVar_t *deep_str = LuaScript_Get(script, "deep_table.pure_tables.string_table");
    assert(deep_str[0].val == 3.0);
    assert(!strcmp(deep_str[1].str, "A"));
    assert(!strcmp(deep_str[2].str, "B"));
    assert(!strcmp(deep_str[3].str, "C"));

    LuaVar_t *mixed = LuaScript_Get(script, "deep_table.mixed_table.prime");
    assert(!strcmp(mixed[1].str, "apple"));
    assert(!strcmp(mixed[2].str, "36.7"));
    assert(mixed[3].val == 0.0);
    Pass(6);
}

int main(void)
{
    LuaScript_t  *script = LuaScript_Load(NULL, "testfile.lua");

    TestOne(script);
    TestTwo(script);
    TestThree(script);
    TestFour(script);
    TestFive(script);
    TestSix(script);

    printf("\nALL TESTS PASSED\n");
    return 0;
}
