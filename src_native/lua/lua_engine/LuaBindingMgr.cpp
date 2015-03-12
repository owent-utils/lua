#include <cstdlib>

#include "LuaBindingMgr.h"

namespace script
{
    namespace lua
    {
        LuaBindingClassMgrBase::LuaBindingClassMgrBase() {
            LuaBindingMgr::Instance()->lua_states_.push_back(this);
        }

        LuaBindingClassMgrBase::~LuaBindingClassMgrBase() {    
        }

        LuaBindingMgr::LuaBindingMgr() : inited_(false)
        {
        }


        LuaBindingMgr::~LuaBindingMgr()
        {
        }

        int LuaBindingMgr::init(lua_State* L) {
            if (nullptr == L) {
                return -1;
            }

            if (!inited_) {
                for (func_type& fn : auto_bind_list_) {
                    LuaAutoBlock block(L);
                    fn();
                }
            }

            for (auto& cmgr : lua_states_) {
                cmgr->addLuaState(L);
            }

            inited_ = true;
            return 0;
        }

        int LuaBindingMgr::proc(lua_State* L) {
            for (auto& mgr : lua_states_) {
                mgr->proc(L);
            }

            return 0;
        }

        void LuaBindingMgr::addBind(func_type fn){
            auto_bind_list_.push_back(fn);
        }

        LuaBindingWrapper::LuaBindingWrapper(LuaBindingMgr::func_type fn) {
            LuaBindingMgr::Instance()->addBind(fn);
        }

        LuaBindingWrapper::~LuaBindingWrapper() {
        }
    }
}
