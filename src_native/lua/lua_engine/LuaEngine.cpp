#include <cstdlib>
#include <list>
#include <ctime>
#include <cmath>
#include <sstream>

#include "LuaEngine.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "log/LogWrapperMgr.h"

#include "../lua_module/LuaTableExt.h"

namespace script
{
    namespace lua
    {
        extern int LuaProfile_openLib(lua_State *L);

        LuaAutoStats::LuaAutoStats() {
            begin_clock_ = std::chrono::steady_clock::now();
        }

        LuaAutoStats::~LuaAutoStats() {
            auto duration = (std::chrono::steady_clock::now() - begin_clock_);
            LuaEngine::Instance()->addLuaStatTime(
                std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0f
            );
            // LuaEngine::Instance()->addLuaStatTime(abs(end_clock_ - begin_clock_) * 1.0f / CLOCKS_PER_SEC);
        }

        LuaEngine::LuaEngine() : state_(NULL)
        {
            lua_update_stats_.lua_time = 0.0f;
            lua_update_stats_.run_time = 0.0f;
        }


        LuaEngine::~LuaEngine()
        {
        }

        int LuaEngine::addOnInited(std::function<void()> fn) {
            on_inited_.push_back(fn);
            return 0;
        }

        int LuaEngine::init(lua_State * state)
        {
            if (nullptr == state)
            {
                state_ = luaL_newstate();
                luaL_openlibs(state_);
            }
            else
            {
                state_ = state;
            }

            // add 3rdparty librarys
            addExtLib(LuaProfile_openLib);
            addExtLib(LuaTableExt_openLib);
            //addExtLib(luaopen_profiler);
            //addExtLib(luaopen_bit);
            //addExtLib(luaopen_pack);
            //addExtLib(luaopen_mime_core);
            //addExtLib(luaopen_socket_core);
            //addExtLib(luaopen_cjson);
            //addExtLib(luaopen_protobuf_c);

            for(std::function<void()>& fn: on_inited_){
                fn();
            }
            on_inited_.clear();
            return 0;
        }

        void LuaEngine::addExtLib(lua_CFunction regfunc) {
            addExtLib(state_, regfunc);
        }

        void LuaEngine::addExtLib(lua_State* L, lua_CFunction regfunc) {
            regfunc(L);
        }

        void LuaEngine::addSearchPath(const std::string& path, bool is_front) {
            addSearchPath(state_, path, is_front);
        }

        void LuaEngine::addSearchPath(lua_State* L, const std::string& path, bool is_front) {
            lua_getglobal(L, "package");                                        /* L: package */
            lua_getfield(L, -1, "path");                      /* get package.path, L: package path */
            const char* cur_path = lua_tostring(L, -1);
            if (is_front)
                lua_pushfstring(L, "%s/?.lua;%s/?.luac;%s", path.c_str(), path.c_str(), cur_path);      /* L: package path newpath */
            else
                lua_pushfstring(L, "%s;%s/?.lua;%s/?.luac", cur_path, path.c_str(), path.c_str());      /* L: package path newpath */
            lua_setfield(L, -3, "path");                /* package.path = newpath, L: package path */
            lua_pop(L, 2);                                                      /* L: - */
        }

        void LuaEngine::addCSearchPath(const std::string& path, bool is_front) {
            addCSearchPath(state_, path, is_front);
        }

        void LuaEngine::addCSearchPath(lua_State* L, const std::string& path, bool is_front) {
            lua_getglobal(L, "package");                                      /* L: package */
            lua_getfield(L, -1, "cpath");                   /* get package.path, L: package cpath */
            const char* cur_path = lua_tostring(L, -1);
#ifdef WIN32
            if (is_front)
                lua_pushfstring(L, "%s/?.dll;%s", path.c_str(), cur_path);    /* L: package path newpath */
            else
                lua_pushfstring(L, "%s;%s/?.dll", cur_path, path.c_str());    /* L: package path newpath */
#else
            if (is_front)
                lua_pushfstring(L, "%s/?.so;%s", path.c_str(), cur_path);    /* L: package path newpath */
            else
                lua_pushfstring(L, "%s;%s/?.so", cur_path, path.c_str());    /* L: package path newpath */
#endif
            lua_setfield(L, -3, "cpath");           /* package.cpath = newpath, L: package cpath */
            lua_pop(L, 2);
        }

        void LuaEngine::addLuaLoader(lua_CFunction func) {
            addLuaLoader(state_, func);
        }

        void LuaEngine::addLuaLoader(lua_State* L, lua_CFunction func) {
            if (!func) return;

            // stack content after the invoking of the function
            // get loader table
            lua_getglobal(L, "package");                                  /* L: package */
            lua_getfield(L, -1, "loaders");                               /* L: package, loaders */

            // insert loader into index 2
            lua_pushcfunction(L, func);                                   /* L: package, loaders, func */
            for (int i = lua_objlen(L, -2) + 1; i > 2; --i)
            {
                lua_rawgeti(L, -2, i - 1);                                /* L: package, loaders, func, function */
                // we call lua_rawgeti, so the loader table now is at -3
                lua_rawseti(L, -3, i);                                    /* L: package, loaders, func */
            }
            lua_rawseti(L, -2, 2);                                        /* L: package, loaders */

            // set loaders into package
            lua_setfield(L, -2, "loaders");                               /* L: package */

            lua_pop(L, 1);
        }

        bool LuaEngine::runCode(const char* codes)
        {
            return runCode(state_, codes);
        }

        bool LuaEngine::runCode(lua_State* L, const char* codes) {
            lua::LuaAutoStats autoLuaStat;

            return fn::exec_code(L, codes);
        }

        bool LuaEngine::runFile(const char* file_path)
        {
            return runFile(state_, file_path);
        }

        bool LuaEngine::runFile(lua_State* L, const char* file_path) {
            lua::LuaAutoStats autoLuaStat;
            return fn::exec_file(L, file_path);
        }

        lua_State* LuaEngine::getLuaState()
        {
            return state_;
        }

        bool LuaEngine::loadItem(const std::string& path, bool auto_create_table) {
            return loadItem(getLuaState(), path, auto_create_table);
        }

        bool LuaEngine::loadItem(const std::string& path, int table_index, bool auto_create_table) {
            return loadItem(getLuaState(), path, table_index, auto_create_table);
        }


        bool LuaEngine::loadItem(lua_State* L, const std::string& path, bool auto_create_table) {
            lua::LuaAutoStats autoLuaStat;
            return fn::load_item(L, path, auto_create_table);
        }

        bool LuaEngine::loadItem(lua_State* L, const std::string& path, int table_index, bool auto_create_table) {
            lua::LuaAutoStats autoLuaStat;
            return fn::load_item(L, path, table_index, auto_create_table);
        }

        bool LuaEngine::removeItem(const std::string& path) {
            return removeItem(getLuaState(), path);
        }

        bool LuaEngine::removeItem(const std::string& path, int table_index) {
            return removeItem(getLuaState(), path, table_index);
        }


        bool LuaEngine::removeItem(lua_State* L, const std::string& path) {
            lua::LuaAutoStats autoLuaStat;
            return fn::remove_item(L, path);
        }

        bool LuaEngine::removeItem(lua_State* L, const std::string& path, int table_index) {
            lua::LuaAutoStats autoLuaStat;
            return fn::remove_item(L, path, table_index);
        }

        bool LuaEngine::loadGlobalEventTrigger(lua_State* L, const std::string& bind_name) {
            lua::LuaAutoStats autoLuaStat;

            lua_getglobal(L, "utils");
            do {
                if (lua_istable(L, -1)) {
                    lua_getfield(L, -1, "event");
                    lua_remove(L, -2);
                    if (lua_istable(L, -1)) {
                        lua_getfield(L, -1, "global");
                        lua_remove(L, -2);
                        if (lua_istable(L, -1)) {
                            lua_getfield(L, -1, "trigger");
                            lua_pushvalue(L, -2);
                            lua_remove(L, -3);
                            lua_pushlstring(L, bind_name.c_str(), bind_name.size());
                            return lua_isfunction(L, -3);
                        }
                    }
                }
            } while (false);

            // 保证增加两个栈对象
            lua_pushnil(L);
            lua_pushnil(L);
            return false;
        }

        bool LuaEngine::loadGlobalEventTrigger(const std::string& bind_name) {
            return loadGlobalEventTrigger(getLuaState(), bind_name);
        }

        int LuaEngine::getPCallHMsg(lua_State* L) {
            return fn::get_pcall_hmsg(L);
        }

        int LuaEngine::getPCallHMsg() {
            return getPCallHMsg(getLuaState());
        }

        void LuaEngine::updateGlobalTimer(float delta) {
            lua_State* state = getLuaState();
            lua::LuaAutoBlock block(state);
            LuaAutoStats autoLuaStat;

            int hmsg = getPCallHMsg(state);

            lua_getglobal(state, "utils");
            if (lua_istable(state, -1)) {
                lua_getfield(state, -1, "event");
                if (lua_istable(state, -1)) {
                    lua_getfield(state, -1, "update");
                    if (lua_isfunction(state, -1)) {
                        lua_pushnumber(state, delta);

                        if (0 != lua_pcall(state, 1, LUA_MULTRET, hmsg)) {
                            WLOGERROR("[Lua]: %s", luaL_checkstring(state, -1));
                        }
                    }
                }
            }

            lua_update_stats_.run_time += delta;
        }

        void LuaEngine::addLuaStatTime(float delta) {
            lua_update_stats_.lua_time += delta;
        }

        std::pair<float, float> LuaEngine::getAndResetLuaStats() {
            std::pair<float, float> ret = std::make_pair(lua_update_stats_.lua_time, lua_update_stats_.run_time);
            lua_update_stats_.lua_time = lua_update_stats_.run_time = 0.0f;
            return ret;
        }

        void LuaEngine::printStack(lua_State* luaState)
        {
            fn::print_stack(luaState);
        }

        void LuaEngine::printTrackBack(lua_State* luaState)
        {
            printStack(luaState);
            fn::print_traceback(luaState, "");
        }
    }
}
