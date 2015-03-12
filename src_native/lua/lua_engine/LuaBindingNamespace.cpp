#include <cstdlib>
#include <sstream>

#include "LuaBindingNamespace.h"

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include "log/LogWrapper.h"

namespace script
{
    namespace lua
    {
        LuaBindingNamespace::LuaBindingNamespace() : lua_state_(nullptr), base_stack_top_(0), this_ns_(0){
        }

        LuaBindingNamespace::LuaBindingNamespace(const char* namespace_, lua_State* L) : lua_state_(nullptr), base_stack_top_(0), this_ns_(0){
            open(namespace_, L);
        }

        LuaBindingNamespace::LuaBindingNamespace(const char* namespace_, LuaBindingNamespace& ns) {
            lua_state_ = ns.lua_state_;
            this_ns_ = ns.this_ns_;
            base_stack_top_ = 0;
            build_ns_set(namespace_);
            find_ns();
            ns_.insert(ns_.begin(), ns.ns_.begin(), ns.ns_.end());
        }

        LuaBindingNamespace::~LuaBindingNamespace() {
            close();
        }

        LuaBindingNamespace::LuaBindingNamespace(LuaBindingNamespace& ns) : lua_state_(nullptr), base_stack_top_(0), this_ns_(0){
            (*this) = ns;
        }
        
        LuaBindingNamespace& LuaBindingNamespace::operator=(LuaBindingNamespace& ns) {
            this_ns_ = ns.this_ns_;
            ns_ = ns.ns_;
            base_stack_top_ = ns.base_stack_top_;
            lua_state_ = ns.lua_state_;

            ns.base_stack_top_ = 0;
            return (*this);
        }


        bool LuaBindingNamespace::open(const char* namespace_, lua_State* L) {
            close();

            if (NULL == namespace_) {
                return true;
            }

            lua_state_ = L;
            ns_.clear();
            build_ns_set(namespace_);

            base_stack_top_ = base_stack_top_ ? base_stack_top_ : lua_gettop(lua_state_);
            return find_ns();
        }

        void LuaBindingNamespace::close() {
            if (0 != base_stack_top_)
                lua_settop(lua_state_, base_stack_top_);

            base_stack_top_ = 0;
            ns_.clear();
        }

        int LuaBindingNamespace::getNamespaceTable() {
            if (this_ns_)
                return this_ns_;
#ifdef LUA_RIDX_GLOBALS
            base_stack_top_ = base_stack_top_ ? base_stack_top_ : lua_gettop(lua_state_);
            lua_rawgeti(lua_state_, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);
            return this_ns_ = lua_gettop(lua_state_);
#else
            return this_ns_ = LUA_GLOBALSINDEX;
#endif
        }

        LuaBindingNamespace::self_type& LuaBindingNamespace::addConst(const char* const_name, const char* n, size_t s) {
            lua_State* state = getLuaState();
            lua_pushstring(state, const_name);
            lua_pushlstring(state, n, s);
            lua_settable(state, getNamespaceTable());

            return *this;
        }

        lua_State* LuaBindingNamespace::getLuaState() {
            return lua_state_;
        }

        int LuaBindingNamespace::__static_method_wrapper(lua_State *L) {
            static_method fn = reinterpret_cast<static_method>(lua_touserdata(L, lua_upvalueindex(1)));
            if (NULL == fn) {
            	WLOGERROR("lua try to call static method but fn not set.\n");
                return 0;
            }

            return fn(L);
        }

        void LuaBindingNamespace::build_ns_set(const char* namespace_) {
            // strtok(str, ".") is not thread safe
            const char* s = namespace_, *e = namespace_;
            while (*e) {
                if ('.' == *e) {
                    ns_.push_back(std::string(s, e));
                    s = e + 1;
                }

                ++e;
                }
            if (s < e)
                ns_.push_back(std::string(s, e));
        }

        bool LuaBindingNamespace::find_ns() {
            int cur_ns = getNamespaceTable();

            for (std::string& ns : ns_) {
                lua_pushlstring(lua_state_, ns.c_str(), ns.size());
                lua_gettable(lua_state_, cur_ns);
                if (lua_isnil(lua_state_, -1)) {
                    lua_pop(lua_state_, 1);
                    lua_newtable(lua_state_);
                    int top = lua_gettop(lua_state_);
                    lua_pushlstring(lua_state_, ns.c_str(), ns.size());
                    lua_pushvalue(lua_state_, top);
                    lua_settable(lua_state_, cur_ns);
                } 

                if (0 == lua_istable(lua_state_, -1)) {
                    return false;
                }
                cur_ns = lua_gettop(lua_state_);
            }

            this_ns_ = cur_ns;
            return true;
        }
    }
}
