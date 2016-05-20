#ifndef _SCRIPT_LUA_LUATABLEEXT_
#define _SCRIPT_LUA_LUATABLEEXT_

#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

namespace script {
    namespace lua {
        int LuaTableExt_openLib(lua_State *L);
    }
}

#endif
