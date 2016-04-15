--region data.conf_manager.lua
--Author : OWenT
--Date   : 2014/10/29
--启动载入项

local class = require('utils.class')
local loader = require('utils.loader')

local conf_set = class.register('data.conf_data_set')

function conf_set:get_by_table(key)
    if not self or not key then
        return nil
    end
    local len = #key
    local i = 1
    local ret = self.__data

    while ret and i <= len do
        ret = ret[key[i]]
        i = i + 1
    end

    return ret
end

-- 数据集，通过key拉取配置数据
function conf_set:get(...)
    return self:get_by_table({...})
end

-- 数据集，拉取所有配置数据
function conf_set:get_all()
    return self.__data
end

function conf_set:set_by_table(args)
    if not self or not args or #args < 2 then
        return nil
    end

    local i = 1
    local ret = self.__data
    local len = #args - 2
    while i <= len do
        if not ret[args[i]] then
            ret[args[i]] = {}
        end
        ret = ret[args[i]]
        i = i + 1
    end

    ret[args[len + 1]] = args[len + 2]
    return args[len + 2]
end

function conf_set:set(...)
    return self:set_by_table({...})
end


local conf_manager = class.register('data.conf_manager', class.singleton)

conf_manager.__path_rule = '%s'
conf_manager.__list_path = nil
conf_manager.__data = {}

-- 设置路径规则
-- @param rule 路径规则(一定要带一个%s)
-- @note 当读取协议message类型为PROTO的配置时，实际会执行require(string.format(rule, PROTO))
--       比如如果配置放在config目录下,那么这里rule填 config.%s
function conf_manager:set_path_rule(rule)
    self.__path_rule = rule
end

-- 设置配置列表加载文件
-- @param l 配置列表加载文件(一定要带一个%s)
-- @note 当执行reload时，清空完原来的配置后后会重新加载 string.format(rule, PROTO)
function conf_manager:set_list(l)
    self.__list_path = l
end

-- 读取配置数据块
-- @param path 配置数据文件路径，相当于执行require(string.format(路径规则, path))
-- @param data_blocks 二进制数据
-- @param data_collector_fn 插入规则函数function (数据集, key, 配置项)
-- @param kv_fn key提取规则函数function (索引, 配置项) return key列表，可以多个 end
-- @param cfg_set_name 别名，通过pbc_config_manager:get(别名)查找配置，默认和path一样
function conf_manager:load(path, data_collector_fn, kv_fn, cfg_set_name)
    path = string.format(self.__path_rule, tostring(path))
    local tb = loader.load(path)
    if nil == tb then
        log_error('load cfg [%s] failed', path)
        return false
    end

    kv_fn = kv_fn or function(k, v)
        return k
    end

    for k,v in pairs(tb) do
        if 'number' == type(k) then
            log_info('load cfg [%s] success, ver=%s, count=%d', path, v.data_ver, v.count)
        else
            cfg_set_name = cfg_set_name or path
            conf_manager.__data[cfg_set_name] = conf_manager.__data[cfg_set_name] or conf_set.new({__data = {}})
            local cfg = conf_manager.__data[cfg_set_name]
            for ck, cv in ipairs(v) do
                local rv = cv
                local rk = { kv_fn(ck, rv) }
                data_collector_fn(cfg, rk, rv)
            end
        end
    end

    -- 释放资源
    loader.remove(path)

    return true
end

-- 读取Key-Value型配置数据块
-- @param path 配置数据文件路径，相当于执行require(string.format(路径规则, path))
-- @param data_blocks 二进制数据
-- @param kv_fn key提取规则函数function (索引, 配置项) return key列表，可以多个 end
-- @param cfg_set_name 别名，通过pbc_config_manager:get(别名)查找配置，默认和path一样
function conf_manager:load_kv(path, kv_fn, cfg_set_name)
    return self:load(path,
        function(cfg, rk, rv)
            if cfg:get_by_table(rk) then
                for i = 1, #rk, 1 do
                    if 0 ~= rk[i] and "" ~= rk[i] and nil ~= rk[i] then
                        log_warn('config [%s] already has key %s, old record will be covered', path, table.concat(rk, ', '))
                        break
                    end
                end
            end

            table.insert(rk, rv)
            cfg:set_by_table(rk)
        end, kv_fn, cfg_set_name
    )
end

-- 读取Key-List型配置数据块
-- @param path 配置数据文件路径，相当于执行require(string.format(路径规则, path))
-- @param data_blocks 二进制数据
-- @param kv_fn key提取规则函数function (索引, 配置项) return key列表，可以多个 end
-- @param cfg_set_name 别名，通过pbc_config_manager:get(别名)查找配置，默认和path一样
function conf_manager:load_kl(path, kv_fn, cfg_set_name)
    return self:load(path,
        function(cfg, rk, rv)
            local ls = cfg:get_by_table(rk)
            if ls then
                table.insert(ls, rv)
            else
                ls = { rv }
                table.insert(rk, ls)
                cfg:set_by_table(rk)
            end
        end, kv_fn, cfg_set_name
    )
end

-- 设置只读
-- @note 实际上就是包装一层然后让__newindex报错,会影响pairs、ipairs等的使用
function conf_manager:set_readonly()
    self.__data = class.set_readonly(self.__data)
end

-- 根据别名(路径)获取配置集合
-- @param type_name 别名(路径)
-- @return 配置集或nil
function conf_manager:get(type_name)
    return self.__data[type_name] or nil
end

-- 重新加载配置并重新加载“列表加载文件”
function conf_manager:reload()
    self.__data = {}
    if self.__list_path and #self.__list_path > 0 then
        loader.remove(self.__list_path)
        loader.load(self.__list_path)
    end
end

return conf_manager
