#pragma once

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

namespace script {
    namespace lua {
        int LuaTableExt_openLib(lua_State *L);
    }
}
