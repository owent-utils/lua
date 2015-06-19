#pragma once

#include <stdint.h>
#include <map>

namespace script {
    namespace binding {

        template<typename TOBJ>
        class LuaObjWithID {
        public:
            typedef TOBJ object_type;
            typedef LuaObjWithID<object_type> self_type;
            typedef self_type* value_type;


        protected:
            LuaObjWithID() {
                while (true) {
                    id_ = ++id_alloc_;
                    if (0 == id_)
                        continue;

                    if (id_mgr_.end() != id_mgr_.find(id_))
                        continue;

                    break;
                }

                id_mgr_[id_] = this;
            }

            virtual ~LuaObjWithID() {
                id_mgr_.erase(id_);
            }

         public:
            uint64_t id() const {
                return id_;
            }

            static object_type* findByID(uint64_t id) {
                typename std::map<uint64_t, value_type>::iterator iter = id_mgr_.find(id);
                if (id_mgr_.end() == iter)
                    return NULL;

                return static_cast<object_type*>(iter->second);
            }

        private:
            uint64_t id_;
            static uint64_t id_alloc_;
            static std::map<uint64_t, value_type> id_mgr_;
        };


        template<typename TOBJ>
        uint64_t LuaObjWithID<TOBJ>::id_alloc_ = 0;

        template<typename TOBJ>
        std::map<uint64_t, typename LuaObjWithID<TOBJ>::value_type > LuaObjWithID<TOBJ>::id_mgr_;
    }
}