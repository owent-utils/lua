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

function conf_set:get(...)
    return self:get_by_table({...})
end

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
conf_manager.__data = {}

function conf_manager:set_path_rule(rule)
    self.__path_rule = rule
end

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

function conf_manager:load_kv(path, kv_fn)
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
        end, kv_fn
    )
end

function conf_manager:load_kl(path, kv_fn)
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
        end, kv_fn
    )
end

function conf_manager:set_readonly()
    self.__data = class.set_readonly(self.__data)
end

function conf_manager:get(type_name)
    return self.__data[type_name] or nil
end

function conf_manager:reload()
    self.__data = {}
    loader.remove('data.conf_list')
    loader.load('data.conf_list')
end

return conf_manager
