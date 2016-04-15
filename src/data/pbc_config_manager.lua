--region data.pbc_config_manager.lua
--Author : OWenT
--Date   : 2015/01/17
--启动载入项

local class = require('utils.class')
local loader = require('utils.loader')

local conf_set = class.register('data.pbc_config_data_set')

-- 必须保证pbc已经载入
local pbc = protobuf

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


local pbc_config_manager = class.register('data.pbc_config_manager', class.singleton)

pbc_config_manager.__path_rule = '%s'
pbc_config_manager.__list_path = nil
pbc_config_manager.__data = {}

-- 设置路径规则
-- @param rule 路径规则(一定要带一个%s)
-- @note 当读取协议message类型为PROTO的配置时，实际查找的协议名称为string.format(rule, PROTO)
--       比如protobuf的package名称是config,那么这里rule填 config.%s
function pbc_config_manager:set_path_rule(rule)
    self.__path_rule = rule
end

-- 设置配置列表加载文件
-- @param l 配置列表加载文件(一定要带一个%s)
-- @note 当执行reload时，清空完原来的配置后后会重新加载 string.format(rule, PROTO)
function pbc_config_manager:set_list(l)
    self.__list_path = l
end

-- 读取配置数据块
-- @param path 配置协议名称
-- @param data_blocks 二进制数据
-- @param data_collector_fn 插入规则函数function (数据集, key, 配置项)
-- @param kv_fn key提取规则函数function (索引, 配置项) return key列表，可以多个 end
-- @param cfg_set_name 别名，通过pbc_config_manager:get(别名)查找配置，默认和path一样
function pbc_config_manager:load_datablocks(path, data_blocks, data_collector_fn, kv_fn, cfg_set_name)
    cfg_set_name = cfg_set_name or path
    pbc_config_manager.__data[cfg_set_name] = pbc_config_manager.__data[cfg_set_name] or conf_set.new({__data = {}})
    local cfg = pbc_config_manager.__data[cfg_set_name]

    path = string.format(self.__path_rule, tostring(path))

    kv_fn = kv_fn or function(k, v)
        return k
    end

    for ck, cv in ipairs(data_blocks.data_block) do
        -- log_debug('config content %s', string.gsub(cv, '.', function(x) return string.format('%02X', string.byte(x)) end) )
        local rv, error_text = pbc.decode(path, cv)

        if false == rv then
            log_error('decode config item failed, path=%s: %s', tostring(path), error_text)
            return false
        end

        -- pbc 解包扩展
        if 'function' == type(pbc.extract) then
            pbc.extract(rv)
        end

        local rk = { kv_fn(ck, rv) }
        data_collector_fn(cfg, rk, rv)
    end

    return true
end

-- 读取Key-Value型配置数据块
-- @param path 配置协议名称
-- @param data_blocks 二进制数据
-- @param kv_fn key提取规则函数function (索引, 配置项) return key列表，可以多个 end
-- @param cfg_set_name 别名，通过pbc_config_manager:get(别名)查找配置，默认和path一样
function pbc_config_manager:load_buffer_kv(path, buffers, kv_fn, cfg_set_name)
    local msg, error_text = pbc.decode("com.owent.xresloader.pb.xresloader_datablocks", buffers)
    if false == msg then
        log_error('decode buffer failed, path=%s: %s', tostring(path), error_text)
        return false
    end

    return self:load_datablocks(path, msg,
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
-- @param path 配置协议名称
-- @param data_blocks 二进制数据
-- @param kv_fn key提取规则函数function (索引, 配置项) return key列表，可以多个 end
-- @param cfg_set_name 别名，通过pbc_config_manager:get(别名)查找配置，默认和path一样
function pbc_config_manager:load_buffer_kl(path, buffers, kv_fn, cfg_set_name)
    local msg, error_text = pbc.decode("com.owent.xresloader.pb.xresloader_datablocks", buffers)
    if false == msg then
        log_error('decode buffer failed, path=%s: %s', tostring(path), error_text)
        return false
    end

    return self:load_datablocks(path, msg,
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
function pbc_config_manager:set_readonly()
    self.__data = class.set_readonly(self.__data)
end

-- 根据别名(路径)获取配置集合
-- @param type_name 别名(路径)
-- @return 配置集或nil
function pbc_config_manager:get(type_name)
    return self.__data[type_name] or nil
end

-- 重新加载配置并重新加载“列表加载文件”
function pbc_config_manager:reload()
    self.__data = {}

    if self.__list_path and #self.__list_path > 0 then
        loader.remove(self.__list_path)
        loader.load(self.__list_path)
    end
end

return pbc_config_manager
