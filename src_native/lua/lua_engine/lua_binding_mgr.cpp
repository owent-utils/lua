#include <cstdlib>

#include "lua_binding_mgr.h"

namespace script {
    namespace lua {
        lua_binding_class_mgr_base::lua_binding_class_mgr_base() { lua_binding_mgr::instance()->lua_states_.push_back(this); }

        lua_binding_class_mgr_base::~lua_binding_class_mgr_base() {}

        lua_binding_mgr::lua_binding_mgr() : inited_(false) {}


        lua_binding_mgr::~lua_binding_mgr() {}

        int lua_binding_mgr::init(lua_State *L) {
            if (nullptr == L) {
                return -1;
            }

            if (!inited_) {
                for (func_type &fn : auto_bind_list_) {
                    lua_auto_block block(L);
                    fn();
                }
            }

            for (auto &cmgr : lua_states_) {
                cmgr->add_lua_state(L);
            }

            inited_ = true;
            return 0;
        }

        int lua_binding_mgr::proc(lua_State *L) {
            for (auto &mgr : lua_states_) {
                mgr->proc(L);
            }

            return 0;
        }

        void lua_binding_mgr::add_bind(func_type fn) { auto_bind_list_.push_back(fn); }

        lua_binding_wrapper::lua_binding_wrapper(lua_binding_mgr::func_type fn) { lua_binding_mgr::instance()->add_bind(fn); }

        lua_binding_wrapper::~lua_binding_wrapper() {}
    }
}
