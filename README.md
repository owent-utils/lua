# OWenT’s Utils - Lua

Lua 增强功能和Lua类型绑定

**注：某次重构更换了命名方式，不再使用驼峰式命名，同意采用小写+下划线分隔单词，其他的风格设定见[.clang-format](.clang-format)文件**

依赖 [atframe_utils][1] 的部分功能。可以参考 [sample/CMakeLists.txt](sample/CMakeLists.txt) 来设置 [atframe_utils][1] ，也可以手动编译 [atframe_utils][1] 的预编译包然后手动添加 [atframe_utils][1] 的包含路径和链接 [atframe_utils][1] 库文件。

```bash
# atframe_utils 参考预编译命令
git clone --depth=100 -b master https://github.com/atframework/atframe_utils.git
mkdir build_dir_atframe_utils
cd build_dir_atframe_utils
cmake ../atframe_utils -DCMAKE_INSTALL_PREFIX=<INSTALL PREFIX>
cmake --build . -j
cmake --build . -- install
export CXXFLAGS="$CXXFLAGS -I<INSTALL PREFIX>/include"
export LDFLAGS="$CXXFLAGS -L<INSTALL PREFIX>/lib -L<INSTALL PREFIX>/lib64 -latframe_utils"

gcc/clang ... <your code files...> $CXXFLAGS $LDFLAGS
```

有任何意见或建议请 [mailto:owt5008137@live.com](mailto:owt5008137@live.com) 或 [mailto:admin@owent.net](mailto:admin@owent.net)

## sample

[sample](sample)里是示例代码，测试了主要功能的api，所以代码比较多。依赖：

+ cmake 3.12 or upper
+ git (with https support)
+ lua/luajit devel/lib (lua 5.1， 5.2， 5.3 都可以)

Sample编译参考命令:

```bash
mkdir build_jobs_sample
cd build_jobs_sample
cmake ../sample
cmake --build . -j
bin/sample sample.lua
```

lua_module里的模块部分依赖C++11或C++0x，不需要的话直接移除即可。

额外模块:

1. ```lua_module/lua_adaptor.h```  Lua版本适配文件
2. ```lua_module/lua_profile.*```  Lua性能主动分析工具
3. ```lua_module/lua_table_ext.*```  Lua table扩展
  > 增加了表克隆table.clone函数，表合并函数 ```table.extend``` ,表递归合并函数 ```table.extend_r``` ,表预分配创建函数 ```table.create``` 等

4. ```lua_module/lua_time_ext.*```  Lua time扩展
  > 增加了毫秒级时间戳函数time_ext.now_ms和微秒级时间戳函数 ```time_ext.now_us``` 并做了防溢出保护

[1]: https://github.com/atframework/atframe_utils
