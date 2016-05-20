local loader = require('utils.loader')

loader.load('main')

print('============================ vardump(game.logic) ============================')
vardump(game.logic)

print('============================ [C].auto_call ============================')
_G.test = {
    auto_call = function(...)
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
obj:print_map({
    test = "ya",
    so = "happy"
})
