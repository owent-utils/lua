--region utils.geometric.lua
--Author : OWenT
--Date   : 2014/10/23
--事件系统

local class = require ('utils.class')

local event = class.register('utils.event', class.singleton)
event.TIMER_TYPE = {
    ONCE = 0,
    LOOP = 1,
}

local dispatcher = class.register('utils.event.dispatcher')

-- 注入new事件，采用预分配table，减少rehash
do
    local old_new = dispatcher.new
    dispatcher.new = function()
        return old_new({
            listener = table.create(0, 32),
            once = table.create(0, 64),
            timer = table.create(0, 64),
        })
    end
end

function dispatcher:add_listener(bind_name, callback, id)
    if nil == self then
        log_error('add_listener require self')
        return nil
    end

    if nil == bind_name or nil == callback then
        log_error('add_listener require bind_name(%s) and callback', bind_name)
        return nil
    end

    id = id or class.alloc_id()

    -- log_debug('%s add listener %s listener[%s]', tostring(self), bind_name, id)
    self.listener[bind_name] = self.listener[bind_name] or table.create(0, 32)
    self.listener[bind_name][id] = callback
    return id
end

function dispatcher:add_once(bind_name, callback, id)
    if nil == bind_name or nil == callback then
        return nil
    end

    id = id or class.alloc_id()

    self.once[bind_name] = self.once[bind_name] or table.create(0, 64)
    self.once[bind_name][id] = callback
    return id
end

function dispatcher:add_timer(time_, callback, type_, timer_name)
    if nil == callback then
        return nil
    end

    type_ = type_ or event.TIMER_TYPE.ONCE
    local id = timer_name or class.alloc_id()

    self.timer[id] = {
        type = type_,
        fn = callback,
        time = time_,
        left = time_,
    }
    return id
end

function dispatcher:update(delta, ...)
    local km = table.create(32, 0)
    for k, v in pairs(self.timer) do
        v.left = v.left - delta
        if v.left <= 0 then
            v.fn(v.time - v.left, ...)
            if v.type == event.TIMER_TYPE.ONCE then
                table.insert(km, k)
            else
                v.left = v.time
            end
        end
    end

    -- 移除只触发一次的定时器
    for k, v in ipairs(km) do
        self.timer[v] = nil
    end
end


-- 移除监听事件
function dispatcher:remove_listener(bind_name, id)
    -- 先尝试移除临时监听器
    if nil ~= self.once[bind_name] then
        local ret = self.once[bind_name][id] or nil
        self.once[bind_name][id] = nil
        return ret
    end

    -- 再尝试移除永久监听器
    if nil ~= self.listener[bind_name] then
        local ret = self.listener[bind_name][id] or nil
        self.listener[bind_name][id] = nil
        return ret
    end

    return nil
end

-- 清空监听事件
function dispatcher:remove_all_listener()
    self.listener = table.create(0, 32)
    self.once = table.create(0, 64)
    return nil
end

-- 移除定时器
function dispatcher:remove_timer(id)
    local ret = self.timer[id] or nil
    self.timer[id] = nil
    return ret
end

-- 清空定时器
function dispatcher:remove_all_timer()
    self.timer = table.create(0, 64)
    return nil
end

function dispatcher:trigger(bind_name, ...)
    local flag = true
    if flag and nil ~= self.listener[bind_name] then
        for k, v in pairs(self.listener[bind_name]) do
            local ret = v(...)
            -- 允许打断
            if 'boolean' == type(ret) and false == ret then
                flag = false
                break;
            end
        end
    end

    if flag and nil ~= self.once[bind_name] then
        for k, v in pairs(self.once[bind_name]) do
            local ret = v(...)
            -- 允许打断
            if 'boolean' == type(ret) and false == ret then
                flag = false
                break;
            end
        end

        self.once[bind_name] = table.create(0, 64)
    end
end

-- 事件管理器
function event.init(obj)
    rawset(obj, '__event', dispatcher.new())
    rawset(obj, 'event', function(self)
        return self.__event
    end)

    return obj
end


function event.add_listener(obj, ...)
    if 'table' ~= type(obj) or 'userdata' ~= type(obj) then
        log_error('event.add_listener require parameter 1 to be a table or userdata')
        return nil
    end

    if nil == rawget(obj, '__event') then
        event.init(obj)
    end

    return obj.__event:add_listener(...)
end

function event.add_once(obj, ...)
    if 'table' ~= type(obj) or 'userdata' ~= type(obj) then
        log_error('event.add_once require parameter 1 to be a table or userdata')
        return nil
    end

    if nil == rawget(obj, '__event') then
        event.init(obj)
    end

    return obj.__event:add_once(...)
end

function event.remove_listener(obj, bind_name, id)
    if 'table' ~= type(obj) or 'userdata' ~= type(obj) then
        log_error('event.remove_listener require parameter 1 to be a table or userdata')
        return nil
    end

    if nil == obj or nil == obj.__event then
        return nil
    end

    return obj.__event:remove_listener(bind_name, id)
end

function event.trigger(obj, bind_name, ...)
    if 'table' ~= type(obj) or 'userdata' ~= type(obj) then
        log_error('event.trigger require parameter 1 to be a table or userdata')
        return nil
    end

    if nil == obj or nil == obj.__event then
        log_error('trigger object event but object or object.__event is nil')
        return
    end

    obj.__event:trigger(bind_name, ...)
end

function event.add_timer(obj, ...)
    if 'table' ~= type(obj) or 'userdata' ~= type(obj) then
        log_error('event.add_timer require parameter 1 to be a table or userdata')
        return nil
    end

    if nil == rawget(obj, '__event') then
        event.init(obj)
    end

    return obj.__event:add_timer(...)
end

-- 全局事件对象
event.global = dispatcher.new()
function event.global:add_listener(bind_name, callback, id)
    if nil == id then
        log_error('add listener to utils.event.global without id is deny to reduce memory leak')
        return nil
    end

    return dispatcher.add_listener(self, bind_name, callback, id)
end

-- 不允许全局定时器必须是具名定时器，防止滥用导致内存泄露
function event.global:add_timer(time_, callback, type_, timer_name)
    if timer_name then
        return dispatcher.add_timer(self, time_, callback, type_, timer_name)
    end

    log_error('add timer to utils.event.global require timer name')
    return nil
end


function event.update(delta)
    event.global:update(delta)
    --collectgarbage('collect')
end

-- 注册到全局
_G.utils = _G.utils or {}
_G.utils.event = event

return event
--endregion
