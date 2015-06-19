#include <cstdlib>
#include <list>
#include <ctime>
#include <cmath>
#include <sstream>
#include <fstream>

#include <String/StringCommon.h>

#include "LuaEvent.h"
#include "../lua_engine/LuaEngine.h"

extern "C" {
#include "lualib.h"
}
namespace script
{
    namespace lua
    {
        int LuaEvent_openLib(lua_State *L) {
            if (false == LuaEngine::loadItem(L, "utils.event", true)) {
                fprintf(stderr, "load utils.event as lua table failed\n");
                return 0;
            }

            return 0;
        }
    }
}
