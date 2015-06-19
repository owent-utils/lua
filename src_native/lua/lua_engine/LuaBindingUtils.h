#pragma once

#include <string>
#include <cstdio>
#include <typeinfo>
#include <memory>

#include <std/explicit_declare.h>

#include "log/LogWrapperMgr.h"

#include "../lua_module/LuaAdaptor.h"

namespace script {
    namespace lua {

        class LuaAutoBlock
        {
        public:
            LuaAutoBlock(lua_State*);

            ~LuaAutoBlock();

            void NullCall();

        private:
            LuaAutoBlock(const LuaAutoBlock& src) FUNC_DELETE;
            const LuaAutoBlock& operator=(const LuaAutoBlock& src) FUNC_DELETE;

            lua_State* m_state;
            int m_stackTop;
        };

        template<typename TC>
        struct LuaBindingUserdataInfo {
            typedef TC value_type;
            typedef std::shared_ptr<value_type> pointer_type;
            typedef std::weak_ptr<value_type> userdata_type;
            typedef userdata_type* userdata_ptr_type;

            static const char* getLuaMetaTableName() {
                return typeid(value_type).name();
            }
        };

        /**
         * 特殊实例（整个对象直接存在userdata里）
         *
         * @tparam  TC  Type of the tc.
         */

        template<typename TC>
        struct LuaBindingPlacementNewAndDelete {
            typedef TC value_type;

            static void pushMetatable(lua_State *L) {
                const char* class_name = LuaBindingUserdataInfo<value_type>::getLuaMetaTableName();
                luaL_getmetatable(L, class_name);
                if (lua_istable(L, -1))
                    return;

                lua_pop(L, 1);
                luaL_newmetatable(L, class_name);
                lua_pushcfunction(L, __lua_gc);
                lua_setfield(L, -2, "__gc");
            }

            /**
             * 创建新实例，并把相关的lua对象入栈
             *
             * @param [in,out]  L   If non-null, the lua_State * to process.
             *
             * @return  null if it fails, else a value_type*.
             */

            template<typename... TParams>
            static value_type* create(lua_State *L, TParams&&... params) {
                value_type* obj = new (lua_newuserdata(L, sizeof(value_type))) value_type(std::forward<TParams>(params)...);

                pushMetatable(L);
                lua_setmetatable(L, -2);
                return obj;
            }

            /**
             * 垃圾回收方法
             *
             * @author  
             * @date    2014/10/25
             *
             * @param [in,out]  L   If non-null, the lua_State * to process.
             *
             * @return  An int.
             */

            static int __lua_gc(lua_State *L) {
                if (0 == lua_gettop(L)) {
                    WLOGERROR("userdata __gc is called without self");
                    return 0;
                }

                // metatable表触发
                if (0 == lua_isuserdata(L, 1)) {
                    lua_remove(L, 1);
                    return 0;
                }

                auto obj = static_cast<value_type*>(luaL_checkudata(L, 1, LuaBindingUserdataInfo<value_type>::getLuaMetaTableName()));
                if (nullptr == obj) {
                    WLOGERROR("try to convert userdata to %s but failed", LuaBindingUserdataInfo<value_type>::getLuaMetaTableName());
                    return 0;
                }
                // 执行析构，由lua负责释放内存
                obj->~value_type();
                return 0;
            }
        };


        namespace fn {
            int get_pcall_hmsg(lua_State* L);

            bool load_item(lua_State* L, const std::string& path, bool auto_create_table = false);

            bool load_item(lua_State* L, const std::string& path, int table_index, bool auto_create_table = false);

            bool remove_item(lua_State* L, const std::string& path);

            bool remove_item(lua_State* L, const std::string& path, int table_index);

            void print_stack(lua_State* L);

            void print_traceback(lua_State* L, const std::string& msg);

            bool exec_file(lua_State* L, const char* file_path);

            bool exec_code(lua_State* L, const char* codes);

            int lua_stackdump(lua_State* L);

            std::string lua_stackdump_to_string(lua_State* L);
        }
    }
}
