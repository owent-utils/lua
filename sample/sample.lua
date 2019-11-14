local loader = require('utils.loader')

loader.load('main')

print('============================ vardump(game.logic) ============================')
vardump(game.logic)

print('============================ [C].auto_call ============================')
_G.test = {
    auto_call = function(...)
        print('start run auto_call')
        vardump({...})
    end
}
game.logic.auto_call()

print('============================ new sample_class ============================')
local obj = game.logic.sample_class.new()
print(string.format('obj:get_m() = %d', obj:get_m()))
print(string.format('obj:state() = %d', obj:state()))
print(string.format('obj:set_m(123)'))
obj:set_m(123)
print(string.format('obj:get_m() = %d', obj:get_m()))
print(string.format('obj:state() = %d', obj:state()))

print('============================ new sample_class.member ============================')
obj:print_vec({3,1,4,1,5,9,2,6,5,3,5,9})
-- static function has no self
local tb = obj.print_map({
    test = "ya",
    so = "happy"
})


print('============================ clone table ============================')
local ntb = table.clone(tb)
print(ntb)
print(tb)

print('============================ extend table ============================')
local ntb = table.extend(ntb, _G.test)
vardump(tb)
vardump(ntb)


print('============================ time_ext ============================')
print(string.format('time_ext.now_ms() = %d', time_ext.now_ms()))
print(string.format('time_ext.now_us() = %d', time_ext.now_us()))