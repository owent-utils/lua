do
    local info = '============================= Load Lua Start ============================='
    if _G.jit then
        info = info .. '\n========== Lua JIT Version:' .. tostring(_G.jit.version) .. ' ==========\n\n\n'
    end

    if _G.lua_log then
        lua_log(0, 1, info)
    else
        print(info)
    end
end

math.randomseed(os.time())

local loader = require('utils.loader')
require('utils.event')

-- 那啥cocos2d会关闭标准输入输出函数, 会导致vardump无输出
do
    -- _G.vardump_default.ostream = log_stream(0, logc_fatal)
end

-- 注册所有game下的table为命名空间
do
    local class = loader.load('utils.class')
    local function reg_game_namespace(tb, prefix, name, base_type)
        local this_path = prefix .. name
        prefix = this_path .. '.'

        for k, v in pairs(tb) do
            -- 命名空间注册基类
            if 'table' == type(v) and nil == getmetatable(v) then
                reg_game_namespace(v, prefix, k, class.namespace)
            end

            -- native类注册基类
            if 'table' == type(v) and nil ~= getmetatable(v) and nil == v.__index then
                reg_game_namespace(v, prefix, k, class.native)
            end
        end

        class.register(tb, base_type, this_path)
    end

    reg_game_namespace(game, '', 'game', class.namespace)
end

-- 加载bootstrap
loader.load_list('bootstrap')


--require "script/testunit/mainMenu"
log_info('============================= Load Lua End =============================')
