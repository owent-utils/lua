#pragma once

#include <string>
#include "DesignPattern/Singleton.h"

extern "C" {
#include "lua.h"
}

namespace script {
    namespace binding {

        class LuaConfManager : public Singleton<LuaConfManager>
        {
        protected:
            LuaConfManager();
            ~LuaConfManager();

        public:

            /**
             * Gets by key and push into lua stack
             *
             * @param [in,out]  L   If non-null, the lua_State* to process.
             * @param   type_name   Name of the type.
             * @param   key         The key.
             *
             * @return  true if it succeeds, false if it fails.
             */

            bool getByKey(lua_State* L, const std::string& type_name, const std::string& key);

            /**
             * Gets by key and push into lua stack
             *
             * @param [in,out]  L   If non-null, the lua_State* to process.
             * @param   type_name   Name of the type.
             * @param   key         The key.
             *
             * @return  true if it succeeds, false if it fails.
             */

            bool getByKey(lua_State* L, const std::string& type_name, int key);

        private:

            /**
             * Gets conf data set and push into lua stack
             *
             * @param [in,out]  L   If non-null, the lua_State* to process.
             * @param   type_name   Name of the type.
             *
             * @return  true if it succeeds, false if it fails.
             */

            bool get_conf_data_set(lua_State* L, const std::string& type_name, int hmsg);
        };

    }
}
