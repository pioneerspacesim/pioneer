
#   include "oolua_check_result.h"
#   include "lua_includes.h"
#   include "oolua_error.h"
#	include "oolua_config.h"

namespace OOLUA
{
    namespace INTERNAL
    {
        bool protected_call_check_result(lua_State* l,int pcall_result)
        {
            if(pcall_result == 0)return true;

#if OOLUA_STORE_LAST_ERROR == 1
            set_error_from_top_of_stack_and_pop_the_error(l);
#elif OOLUA_USE_EXCEPTIONS == 1
            if( pcall_result == LUA_ERRRUN)
                throw OOLUA::Runtime_error(l,(OOLUA::ERROR::PopTheStack*)0);
            else if(pcall_result == LUA_ERRMEM)
                throw OOLUA::Memory_error(l,(OOLUA::ERROR::PopTheStack*)0);
            else if(pcall_result == LUA_ERRERR)
			throw OOLUA::Runtime_error(l,(OOLUA::ERROR::PopTheStack*)0);
#elif OOLUA_DEBUG_CHECKS == 1
            (void)l;
            if( pcall_result == LUA_ERRRUN)
                assert(0 && "LUA_ERRRUN");
            else if(pcall_result == LUA_ERRMEM)
                assert(0 && "LUA_ERRMEM");
            else if(pcall_result == LUA_ERRERR)
                assert(0 && "LUA_ERRERR");
#else
			(void)l;
#endif
            return false;
        }


        bool load_buffer_check_result(lua_State* l,int result)
        {
            if(result == 0)return true;
#if OOLUA_STORE_LAST_ERROR == 1
            set_error_from_top_of_stack_and_pop_the_error(l);
#elif OOLUA_USE_EXCEPTIONS == 1
            if(result == LUA_ERRFILE)
                throw OOLUA::File_error(l,(OOLUA::ERROR::PopTheStack*)0);
            else if(result == LUA_ERRSYNTAX)
                throw OOLUA::Syntax_error(l,(OOLUA::ERROR::PopTheStack*)0);
            else if(result == LUA_ERRMEM )
                throw OOLUA::Memory_error(l,(OOLUA::ERROR::PopTheStack*)0);
#elif OOLUA_DEBUG_CHECKS == 1
            (void)l;
            if(result == LUA_ERRSYNTAX)
                assert(0 && "syntax error");
            else if(result == LUA_ERRMEM)
                assert(0 && "memory error");
            else if(result == LUA_ERRFILE)
                    assert(0 && "file error");
#else
			(void)l;
#endif
            return false;
        }
    }
}

