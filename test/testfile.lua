global_number = 10
global_string = "Test String!"
global_true_bool = true
global_false_bool = false

nested_table = { number = 666, string = 'Loaded the nested String!', bool = true }

global_array = {1, 2, 3, 4}
nested_array = { nums = { 5, 6, 7, 8 } }

deep_table = { pure_tables = {string_table = {'A', 'B', 'C'},
                              num_table = {1, 2, 3},
                              bool_table = {true, false, true}
                             },
                mixed_table = { prime = {"apple", 36.7, false} }
}

-- for _, v in pairs(deep_table.mixed_table.prime) do print(v) end
