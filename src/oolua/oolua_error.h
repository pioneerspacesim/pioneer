#ifndef OOLUA_ERROR_H_
#   define OOLUA_ERROR_H_

// XXX -TM-
#define OOLUA_STORE_ERROR

#ifdef OOLUA_EXCEPTIONS
#   include "oolua_exception.h"
#elif defined OOLUA_STORE_ERROR
#   include <string>
struct lua_State;
namespace OOLUA
{
    void reset_error_value(lua_State*l);
    std::string get_last_error(lua_State*l);

    namespace INTERNAL
    {
        void set_error_from_top_of_stack(lua_State*l);
    }
}
#else
#   include <cassert>
#endif


#endif

