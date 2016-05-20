#ifndef _SCRIPT_LUA_LUATABLEEXT_
#define _SCRIPT_LUA_LUATABLEEXT_

#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

namespace script {
    namespace lua {
        int lua_table_ext_openlib(lua_State *L);
    }
}

#endif
