#pragma once

#include "design_pattern/singleton.h"
#include <assert.h>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

namespace script {
    namespace lua {

        int LuaProfile_openLib(lua_State *L);

        struct LuaProfilePairHash {
        public:
            template <typename T, typename U>
            std::size_t operator()(const std::pair<T, U> &x) const {
                return std::hash<T>()(x.first) ^ std::hash<U>()(x.second);
            }

            template <typename T, typename U>
            bool equal(const std::pair<T, U> &l, const std::pair<T, U> &r) const {
                std::size_t lh = std::hash<T>()(l.first) ^ std::hash<U>()(l.second);
                std::size_t rh = std::hash<T>()(r.first) ^ std::hash<U>()(r.second);

                return (lh == rh) && (l == r);
            }
        };

        struct LuaProfileStatData {
            typedef std::chrono::steady_clock clock_t;

            std::string source; // 源码/文件
            int line_number;    // 起始位置
            std::string name;   // 函数名

            size_t call_count;                      // 调用次数
            clock_t::duration call_full_duration;   // 调用总时钟周期
            clock_t::duration call_inner_duration;  // 调用内部时钟周期
            clock_t::duration call_native_duration; // 调用本地调用时钟周期

            void reset();
        };

        struct LuaProfileStackData {
            typedef LuaProfileStatData::clock_t clock_t;
            typedef std::pair<std::string, int> prof_key_t;
            typedef std::shared_ptr<LuaProfileStackData> stack_ptr_t;
            typedef std::unordered_map<prof_key_t, stack_ptr_t, LuaProfilePairHash> map_t;

            std::string source; // 源码/文件
            int line_number;    // 起始位置
            std::string name;   // 函数名

            size_t call_count;                      // 调用次数
            clock_t::duration call_full_duration;   // 调用总时钟周期
            clock_t::duration call_inner_duration;  // 调用内部时钟周期
            clock_t::duration call_native_duration; // 调用本地调用时钟周期

            map_t children;
            std::weak_ptr<LuaProfileStackData> parent;

            static stack_ptr_t make(const prof_key_t &key);

            static stack_ptr_t enter_fn(stack_ptr_t &self, const prof_key_t &key);

            static stack_ptr_t exit_fn(stack_ptr_t &self);
        };

        struct LuaProfileCallData {
            typedef LuaProfileStackData::prof_key_t prof_key_t;
            typedef LuaProfileStatData::clock_t clock_t;
            typedef std::shared_ptr<LuaProfileStatData> stats_ptr;

            //
            stats_ptr call_stats;

            // 统计记录点
            clock_t::time_point call_enter_time_point;
            // 本次调用统计
            clock_t::duration call_full_duration;  // 调用总时钟周期
            clock_t::duration call_inner_duration; // 调用内部时钟周

            bool is_native_call;
            prof_key_t key;
        };

        class lua_profile : public Singleton<lua_profile> {
        public:
            typedef LuaProfileCallData::prof_key_t prof_key_t;
            typedef LuaProfileCallData::stats_ptr prof_ptr;
            typedef LuaProfileStatData::clock_t clock_t;
            typedef std::unordered_map<prof_key_t, prof_ptr, LuaProfilePairHash> map_t;

        protected:
            lua_profile();
            ~lua_profile();


        public:
            void init(lua_State *L);

            void reset();

            void stop(lua_State *L);

            void enable();
            void disable();

            void enableNativeProf();
            void disableNativeProf();

            std::string dump_to(const std::string &file_path);

            std::string dump();

            size_t push_fn(LuaProfileStackData::stack_ptr_t ptr);
            void pop_fn(size_t p);

        private:
            static void Hook_Fn(lua_State *L, lua_Debug *ar);

            LuaProfileCallData &enter_lua_func(const prof_key_t &key);
            void exit_lua_func(const prof_key_t &key);

            LuaProfileCallData &enter_native_func(const prof_key_t &key);
            void exit_native_func(const prof_key_t &key);

        private:
            bool inited_;
            bool enabled_;
            bool enabled_native_code_;
            lua_Hook origin_hook_;
            int origin_mask_;

            std::vector<LuaProfileCallData> call_stack_; // 调用栈
            map_t call_stats_;

            std::vector<LuaProfileStackData::stack_ptr_t> call_fn_prof_list_;
        };
    }
}
