/**
 * @note 多线程多lua虚拟机情况下，应该在每个线程单独对虚拟机执行LuaBindingMgr::Instance()->proc(L);以防止多线程冲突
 */

#pragma once

#include <string>
#include <assert.h>
#include <stdint.h>
#include <cstddef>
#include <functional>
#include <memory>
#include <list>
#include <map>

#include <DesignPattern/Singleton.h>
#include <std/explicit_declare.h>

#include "LuaBindingUtils.h"

namespace script {
    namespace lua {
        class LuaBindingClassMgrBase {
        protected:
            LuaBindingClassMgrBase();

        public:
            virtual ~LuaBindingClassMgrBase() = 0;

            virtual int proc(lua_State* L) = 0;

            virtual void addLuaState(lua_State* L) = 0;
        };


        template<typename TC>
        class LuaBindingClassMgrInst : public LuaBindingClassMgrBase, public Singleton< LuaBindingClassMgrInst<TC> > {
        public:
            virtual int proc(lua_State* L) CLASS_OVERRIDE{
                if (nullptr == L) {
                    cache_maps_.clear();
                    return 0;
                }

                intptr_t index = reinterpret_cast<intptr_t>(L);
                auto iter = cache_maps_.find(index);
                if (cache_maps_.end() == iter) {
                    return -1;
                }

                iter->second.clear();
                return 0;
            }

                bool addRef(lua_State* L, const std::shared_ptr<TC>& ptr) {
                if (nullptr == L) {
                    return false;
                }

                intptr_t index = reinterpret_cast<intptr_t>(L);
                auto iter = cache_maps_.find(index);
                if (cache_maps_.end() == iter) {
                    return false;
                }

                iter->second.push_back(ptr);
                return true;
            }

            virtual void addLuaState(lua_State* L) CLASS_OVERRIDE{
                intptr_t index = reinterpret_cast<intptr_t>(L);
                cache_maps_[index];
            }

        private:
            std::map<intptr_t, std::list<std::shared_ptr<TC> > > cache_maps_;
        };

        class LuaBindingMgr : public Singleton<LuaBindingMgr> {
        public:
            typedef std::function<void()> func_type;

        protected:
            LuaBindingMgr();
            ~LuaBindingMgr();

        public:
            int init(lua_State* L);

            /** 
             * 自动更新/清理入口
             * @param 指定要清理的lua虚拟机，不指定为清理全部
             */
            int proc(lua_State* L = nullptr);

            void addBind(func_type fn);

        public:
            template<typename TC>
            bool addRef(lua_State* L, const std::shared_ptr<TC>& ptr) {
                return LuaBindingClassMgrInst<TC>::Instance()->addRef(L, ptr);
            }

            inline bool isInited() const { return inited_; }
        private:
            bool inited_;
            std::list<func_type> auto_bind_list_;
            std::list<LuaBindingClassMgrBase*> lua_states_;
            friend class LuaBindingClassMgrBase;
        };


        class LuaBindingWrapper {
        public:
            LuaBindingWrapper(LuaBindingMgr::func_type);
            ~LuaBindingWrapper();
        };
    }
}

#define LUA_BIND_VAR_NAME(name)  script_lua_LuaBindMgr_Var_##name
#define LUA_BIND_FN_NAME(name)  script_lua_LuaBindMgr_Fn_##name

#define LUA_BIND_OBJECT(name)   \
    static void LUA_BIND_FN_NAME(name)(); \
    static script::lua::LuaBindingWrapper LUA_BIND_VAR_NAME(name)(LUA_BIND_FN_NAME(name)); \
    void LUA_BIND_FN_NAME(name)()

