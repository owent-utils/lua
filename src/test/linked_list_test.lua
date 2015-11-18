local linked_list = require('linked_list')

local function print_array(tb)
    print(table.concat(tb, ', '))
end

local tested1 = linked_list.new()

tested1:push_back(4)
tested1:push_front(3)
tested1:push_back(5)
tested1:push_back(6)

tested1:push_front(2)
tested1:push_front(1)



print_array(tested1:to_array())
print(string.format('size = %d', tested1:size()))
print(tostring(tested1))

assert(1 == tested1:pop_front())
assert(2 == tested1:pop_front())
assert(6 == tested1:pop_back())
assert(5 == tested1:pop_back())
assert(2 == tested1:size())
assert(not tested1:empty())
assert(3 == tested1:front())
assert(4 == tested1:back())

assert(tested1:lbegin():next():next() == tested1:lend())
assert(4 == tested1:rbegin():get())
assert(tested1:rbegin():prev():prev() == tested1:rend())
assert(tested1:lend() == tested1:rend())

local tested2 = linked_list.new()

tested2:push_front(8)
tested2:push_front(7)
assert(2 == tested2:size())

tested1:insert(tested1:rbegin(), tested2:lbegin():get())

tested1:rbegin():insert(tested2:lbegin())
assert(1 == tested2:size())
assert(4 == tested1:size())
assert(7 == tested1:rbegin():prev():get())
assert(7 == tested1:rbegin():prev():prev():get())

tested2:lbegin():remove()

assert(0 == tested2:size())
assert(tested2:empty())
print(tostring(tested1))

local benchmark = linked_list.new()
local benckmark_count = 100000
for i = 1, benckmark_count, 1 do
    benchmark:push_back(i)
end

for i = 1, benckmark_count, 1 do
    assert(i == benchmark:pop_front(i))
end

print(string.format('benchmark with count=%d done', benckmark_count))