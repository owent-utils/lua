#include "LuaConfManager.h"

#include "cocos2d.h"
#include "lua/lua_engine/LuaEngine.h"

namespace script {
    namespace binding {
        LuaConfManager::LuaConfManager()
        {
        }


        LuaConfManager::~LuaConfManager()
        {
        }


        bool LuaConfManager::getByKey(lua_State* L, const std::string& type_name, const std::string& key) {
            lua::LuaAutoStats autoLuaStat;

            int top = lua_gettop(L);
            int hmsg = lua::LuaEngine::getPCallHMsg(L);
            bool ret = get_conf_data_set(L, type_name, hmsg);
            if (!ret) {
                lua_remove(L, -2);
                return ret;
            }

            // lua栈 : +2 (function, table)
            lua_getfield(L, -1, "get");
            lua_pushvalue(L, -2);
            lua_pushlstring(L, key.c_str(), key.size());
            if (0 != lua_pcall(L, 2, 1, hmsg)) {
                cocos2d::log("call lua utils.class.get('data.conf_manager'):get('%s'):get('%s') failed.\n%s",
                    type_name.c_str(), key.c_str(), luaL_checkstring(L, -1)
                );
                lua_settop(L, top);
                lua_pushnil(L);
                return false;
            }

            // lua栈 : +3 (function, table, table)
            lua_remove(L, -2);
            lua_remove(L, -2);
            return lua_istable(L, -1);
        }

        bool LuaConfManager::getByKey(lua_State* L, const std::string& type_name, int key) {
            lua::LuaAutoStats autoLuaStat;

            int top = lua_gettop(L);
            int hmsg = lua::LuaEngine::getPCallHMsg(L);
            bool ret = get_conf_data_set(L, type_name, hmsg);
            if (!ret) {
                lua_remove(L, -2);
                return ret;
            }

            // lua栈 : +2 (function, table)
            lua_getfield(L, -1, "get");
            lua_pushvalue(L, -2);
            lua_pushinteger(L, key);
            if (0 != lua_pcall(L, 2, 1, hmsg)) {
                cocos2d::log("call lua utils.class.get('data.conf_manager'):get('%s'):get(%d) failed.\n%s",
                    type_name.c_str(), key, luaL_checkstring(L, -1)
                );
                lua_settop(L, top);
                lua_pushnil(L);
                
                return false;
            }

            // lua栈 : +3 (function, table, table)
            lua_remove(L, -2);
            lua_remove(L, -2);
            return lua_istable(L, -1);
        }

        bool LuaConfManager::get_conf_data_set(lua_State* L, const std::string& type_name, int hmsg) {
            int top = lua_gettop(L);

            // 获取utils.class.get函数
            lua::LuaEngine::loadItem(L, "utils.class.get");
            if (0 == lua_isfunction(L, -1)) {
                lua_settop(L, top);
                lua_pushnil(L);
                cocos2d::log("lua path utils.class.get is invalid");
                return false;
            }
            // lua栈 : +1 (function)

            // 执行utils.class.get('data.conf_manager')
            lua_pushliteral(L, "data.conf_manager");
            if (0 != lua_pcall(L, 1, 1, hmsg)) {
                cocos2d::log("lua call utils.class.get('data.conf_manager').get('%s') failed\n%s", type_name.c_str(), luaL_checkstring(L, -1));
                lua_settop(L, top);
                lua_pushnil(L);
                return false;
            }

            // lua栈 : +1 (table)
            // 
            if (0 == lua_istable(L, -1)) {
                lua_settop(L, top);
                lua_pushnil(L);
                cocos2d::log("lua call utils.class.get('data.conf_manager') got nil");
                return false;
            }

            lua_getfield(L, -1, "get");
            // lua栈 : +2 (top => table, function)
            lua_pushvalue(L, -2);
            // lua栈 : +3 (top => table, function, table)
            lua_pushlstring(L, type_name.c_str(), type_name.size());
            // lua栈 : +4 (top => table, function, table, name)
            
            if (0 != lua_pcall(L, 2, 1, hmsg)) {
                cocos2d::log("lua call utils.class.get('data.conf_manager'):get('%s') failed\n%s", type_name.c_str(), luaL_checkstring(L, -1));
                lua_settop(L, top);
                lua_pushnil(L);
                return false;
            }
            // lua栈 : +2 (top => table[conf_manager], table[conf_data_set])
            
            lua_remove(L, -2);
            return lua_istable(L, -1);
        }

    }
}
