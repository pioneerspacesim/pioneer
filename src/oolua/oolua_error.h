#ifndef OOLUA_ERROR_H_
#   define OOLUA_ERROR_H_

#include "oolua_config.h"

#if OOLUA_USE_EXCEPTIONS == 1
#   include "oolua_exception.h"
#endif

#if OOLUA_DEBUG_CHECKS == 1
#   include <cassert>
#endif



#   include <string>
struct lua_State;
namespace OOLUA
{
    void reset_error_value(lua_State*l);
    std::string get_last_error(lua_State*l);
	
    namespace INTERNAL
    {
        void set_error_from_top_of_stack(lua_State*l);
		void set_error_from_top_of_stack_and_pop_the_error(lua_State*l);
    }
}

#endif

