--region data.list.lua
--Author : OWenT
--Date   : 2014/10/29
--启动载入项

local loader = require('utils.loader')
local cfg = loader.load_kv('data.conf_manager')

-- role_cfg
--cfg:load_kv('unit.role_cfg', function(k, v)
--    return v.id or k
--end)
--
--cfg:load_kv('unit.role_cfg', function(k, v)
--    return v.id or k
--end, 'role_cfg')