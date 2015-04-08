-- added by owent, pbc结构体展开 
-- 需要添加到 pbc的protobuf.lua 中

function extract(tbl)
    local typename = rawget(tbl , 1)
    local buffer = rawget(tbl , 2)
    if type(typename) == "string" and type(buffer) == "string" then
        if check(typename) then
            expand(tbl)
        end
    end

    for k, v in pairs(tbl) do
        if type(v) == "table" then
            extract(v)
        end
    end
end
