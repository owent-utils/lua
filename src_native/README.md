# OWenT’s Utils - Lua 本地代码拓展

依赖 https://github.com/atframework/atframe_utils 中的
+ [include/std](https://github.com/atframework/atframe_utils/tree/master/include/std) 目录
+ [include/design_pattern/singleton.h](https://github.com/atframework/atframe_utils/blob/master/include/design_pattern/singleton.h)
+ [include/design_pattern/noncopyable.h](https://github.com/atframework/atframe_utils/blob/master/include/design_pattern/noncopyable.h)
+ [include/lock](https://github.com/atframework/atframe_utils/tree/master/include/lock) 目录

全部都是头文件依赖，所以如果项目中没有使用[atframe_utils](https://github.com/atframework/atframe_utils)的话，只要下载来头文件即可

有任何意见或建议请 [mailto:owt5008137@live.com](mailto:owt5008137@live.com) 或 [mailto:admin@owent.net](mailto:admin@owent.net)

[sample](sample)里是示例代码，lua 5.1， 5.2， 5.3 都可以

lua_module里的模块部分依赖C++11或C++0x，不需要的话直接移除即可
1. lua_module/lua_adaptor.h  Lua版本适配文件
2. lua_module/lua_profile.*  Lua性能主动分析工具
3. lua_module/lua_table_ext.*  Lua table扩展
> 增加了表克隆table.clone函数，表合并函数table.extend,表递归合并函数table.extend_r,表预分配创建函数table.create等

4. lua_module/lua_time_ext.*  Lua time扩展
> 增加了毫秒级时间戳函数time_ext.now_ms和微秒级时间戳函数time_ext.now_us并做了防溢出保护
