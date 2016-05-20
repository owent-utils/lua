#include <cstddef>
#include <iostream>
#include <map>
#include <stdint.h>
#include <string>
#include <vector>

#include "lua/lua_module/lua_profile.h"
#include "lua/lua_module/lua_table_ext.h"
#include "lua/lua_module/lua_time_ext.h"

#include "lua/lua_engine/lua_binding_class.h"
#include "lua/lua_engine/lua_binding_mgr.h"
#include "lua/lua_engine/lua_binding_namespace.h"
#include "lua/lua_engine/lua_engine.h"

// 测试用的类
class sample_class {
public:
    enum STATE_T { CREATED = 0, INITED };

public:
    sample_class() : m_(0), state_(CREATED) {}
    ~sample_class() {}

    STATE_T state() const { return state_; }

    int get_m() const { return m_; }

    void set_m(int m) {
        m_ = m;
        state_ = INITED;
    }

    void print_vec(const std::vector<int> &vec) {
        std::cout << "print_vec: ";
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i) {
                std::cout << ", ";
            }
            std::cout << vec[i];
        }
        std::cout << std::endl;
    }

    static std::map<std::string, std::string> print_map(const std::map<std::string, std::string> &m) {
        std::cout << "print_map: {" << std::endl;
        for (std::map<std::string, std::string>::const_iterator iter = m.begin(); iter != m.end(); ++iter) {
            std::cout << "\t" << iter->first << " = " << iter->second << std::endl;
        }
        std::cout << "}" << std::endl;

        return m;
    }

private:
    int m_;
    STATE_T state_;
};

// 自定义std::map<std::string, std::string>到luatable的转换规则
namespace script {
    namespace lua {
        namespace detail {
            template <typename... Ty>
            struct wraper_var<std::map<std::string, std::string>, Ty...> {
                static int wraper(lua_State *L, const std::map<std::string, std::string> &v) {
                    lua_createtable(L, 0, static_cast<int>(v.size()));

                    for (std::map<std::string, std::string>::const_iterator iter = v.begin(); iter != v.end(); ++iter) {
                        lua_pushlstring(L, iter->first.c_str(), iter->first.size());
                        lua_pushlstring(L, iter->second.c_str(), iter->second.size());
                        lua_settable(L, -3);
                    }

                    return 1;
                }
            };

            template <typename... Ty>
            struct unwraper_var<std::map<std::string, std::string>, Ty...> {
                static std::map<std::string, std::string> unwraper(lua_State *L, int index) {
                    // script::lua::lua_auto_block protect_top(L);

                    std::map<std::string, std::string> ret;
                    if (lua_gettop(L) < index) {
                        return ret;
                    }

                    luaL_checktype(L, index, LUA_TTABLE);

                    /* table is in the stack at index 't' */
                    lua_pushnil(L); /* first key */
                    while (lua_next(L, index) != 0) {
                        std::string key = luaL_checkstring(L, -2);
                        std::string val = luaL_checkstring(L, -1);
                        ret[key] = val;
                        lua_pop(L, 1);
                    }

                    return std::move(ret);
                }
            };
        }
    }
}

int main(int argc, char *argv[]) {
    script::lua::lua_engine::get_instance().add_on_inited([](lua_State *state) {
        // 初始化完成后加载扩展库
        script::lua::lua_engine::get_instance().add_ext_lib(script::lua::lua_profile_openlib);
        script::lua::lua_engine::get_instance().add_ext_lib(script::lua::lua_table_ext_openlib);
        script::lua::lua_engine::get_instance().add_ext_lib(script::lua::lua_time_ext_openlib);
    });

    // 初始化
    script::lua::lua_engine::get_instance().init();

    // 添加搜索目录
    script::lua::lua_engine::get_instance().add_search_path("../../src");

    // 对象池管理器初始化
    script::lua::lua_binding_mgr::get_instance().init(script::lua::lua_engine::get_instance().get_lua_state());

    // 执行参数中的脚本
    for (int i = 1; i < argc; ++i) {
        script::lua::lua_engine::get_instance().run_file(argv[i]);
    }

    std::string code;
    while (std::getline(std::cin, code)) {
        // 必须UTF-8编码
        script::lua::lua_engine::get_instance().run_code(code.c_str());
        code.clear();

        // 定期调用script::lua::lua_binding_mgr::get_instance().proc(script::lua::lua_engine::get_instance().get_lua_state());
        // 对C++层的对象回收将会在这里进行
        script::lua::lua_binding_mgr::get_instance().proc(script::lua::lua_engine::get_instance().get_lua_state());
    }
    return 0;
}


static void test_namespace_method_and_auto_call() {
    lua_State *L = script::lua::lua_engine::instance()->get_lua_state();

    std::vector<std::string> vec;
    vec.push_back("hello");
    vec.push_back("world");
    vec.push_back("!");
    script::lua::auto_call(L, "_G.test.auto_call", "ready go", 123, vec);
}

LUA_BIND_OBJECT(sample_class) {
    lua_State *L = script::lua::lua_engine::instance()->get_lua_state();

    script::lua::lua_binding_class<sample_class> clazz("sample_class", "game.logic", L);
    // 使用默认的new方法
    // 尖括号中是构造函数的参数类型列表
    // 如果要自定义new方法请使用setNew函数
    clazz.setDefaultNew<>();


    {
        script::lua::lua_binding_namespace cs("sample_class_state_t", clazz.get_owner_namespace());

        // 常量
        cs.add_const("CREATED", sample_class::CREATED);
        cs.add_const("INITED", sample_class::INITED);

        // 函数
        clazz.get_owner_namespace().add_method("auto_call", test_namespace_method_and_auto_call);
    }

    // 函数
    {
        clazz.add_method("state", &sample_class::state);
        clazz.add_method("get_m", &sample_class::get_m);
        clazz.add_method("set_m", &sample_class::set_m);
        clazz.add_method("print_vec", &sample_class::print_vec);
        clazz.add_method("print_map", &sample_class::print_map);
    }
}
