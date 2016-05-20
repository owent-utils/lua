#ifndef _SCRIPT_LUA_LUATIMEEXT_
#define _SCRIPT_LUA_LUATIMEEXT_

#pragma once

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

namespace script {
    namespace lua {
        int lua_time_ext_openlib(lua_State *L);
    }
}

#endif
