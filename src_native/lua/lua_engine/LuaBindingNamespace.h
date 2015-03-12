#pragma once

#include <string>
#include <list>
#include <functional>
#include <std/explicit_declare.h>

#include "LuaBindingUnwrapper.h"
#include "LuaBindingWrapper.h"

namespace script {
    namespace lua {

        /**
         * lua 命名空间，注意只能用于局部变量
         *
         * @author  owent
         * @date    2014/10/25
         */

        template<typename T, typename TP>
        class LuaBindingClass;

        class LuaBindingNamespace {
        public:
            typedef LuaBindingNamespace self_type;
            typedef int(*static_method)(lua_State*);

        public:
            LuaBindingNamespace();
            LuaBindingNamespace(const char* namespace_, lua_State* L);

            /**
             * 在namespace的基础上再构建namespace
             *
             * @param   namespace_  The namespace.
             * @param   ns          The father ns.
             */

            LuaBindingNamespace(const char* namespace_, LuaBindingNamespace& ns);
            ~LuaBindingNamespace();

            /**
             * 开启命名空间
             */
            bool open(const char* namespace_, lua_State* L);

            /**
             * 关闭命名空间，重置lua栈top
             *  
             */
            void close();

            int getNamespaceTable();

            /**
             * 复制构造,会转移命名空间table的所有权
             */
            LuaBindingNamespace(LuaBindingNamespace& ns);

            /**
             * 复制赋值,会转移命名空间table的所有权
             * @param ns 目标
             */
            LuaBindingNamespace& operator=(LuaBindingNamespace& ns);


            /**
            * 添加常量(自动类型推断)
            *
            * @return  self.
            */
            template<typename Ty>
            self_type& addConst(const char* const_name, Ty n) {
                lua_State* state = getLuaState();
                int ret_num = detail::wraper_var<Ty>::wraper(state, n);
                if (ret_num > 0)
                    lua_setfield(state, getNamespaceTable(), const_name);

                return *this;
            }

            /**
            * 添加常量(字符串)
            *
            * @return  self.
            */
            self_type& addConst(const char* const_name, const char* n, size_t s);

            /**
             * 给命名空间添加方法，自动推断类型
             *
             * @tparam  TF  Type of the tf.
             * @param   func_name   Name of the function.
             * @param   fn          The function.
             */
            template<typename R, typename... TParam>
            self_type& addMethod(const char* func_name, R(*fn)(TParam... param)) {
                lua_State* state = getLuaState();
                lua_pushstring(state, func_name);
                lua_pushlightuserdata(state, reinterpret_cast<void*>(fn));
                lua_pushcclosure(state, detail::unwraper_static_fn<R, TParam...>::LuaCFunction, 1);
                lua_settable(state, getNamespaceTable());

                return (*this);
            }

            /**
             * 给命名空间添加仿函数，自动推断类型
             *
             * @tparam  R           Type of the r.
             * @tparam  TParam      Type of the parameter.
             * @param   func_name   Name of the function.
             * @param   fn          functor
             *
             * @return  A self_type&amp;
             */
            template<typename R, typename... TParam>
            self_type& addMethod(const char* func_name, std::function<R(TParam...)> fn) {
                typedef std::function<R(TParam...)> fn_t;

                lua_State* state = getLuaState();
                lua_pushstring(state, func_name);

                LuaBindingPlacementNewAndDelete<fn_t>::create(state, fn);
                lua_pushcclosure(state, detail::unwraper_functor_fn<R, TParam...>::LuaCFunction, 1);
                lua_settable(state, getNamespaceTable());

                return (*this);
            }


            lua_State* getLuaState();
        private:
            static int __static_method_wrapper(lua_State *L);

        private:
            bool find_ns();
            void build_ns_set(const char* namespace_);

            lua_State* lua_state_;
            std::list<std::string> ns_;
            int base_stack_top_;
            int this_ns_;

            template<typename T, typename TP>
            friend class LuaBindingClass;
        };
    }
}
