
#   include "oolua_check_result.h"
#   include "lua_includes.h"
#   include "oolua_error.h"

namespace OOLUA
{
    namespace INTERNAL
    {
        bool protected_call_check_result(lua_State* l,int pcall_result)
        {
            if(pcall_result == 0)return true;

#if defined OOLUA_STORE_ERROR
            set_error_from_top_of_stack(l);
#elif defined OOLUA_EXCEPTIONS
            if( pcall_result == LUA_ERRRUN)
                throw OOLUA::Runtime_error(l);
            else if(pcall_result == LUA_ERRMEM)
                throw OOLUA::Memory_error(l);
            else if(pcall_result == LUA_ERRERR)
			throw OOLUA::Runtime_error(l);
#elif defined DEBUG || defined _DEBUG
            (void)l;
            if( pcall_result == LUA_ERRRUN)
                assert(0 && "LUA_ERRRUN");
            else if(pcall_result == LUA_ERRMEM)
                assert(0 && "LUA_ERRMEM");
            else if(pcall_result == LUA_ERRERR)
                assert(0 && "LUA_ERRERR");
#endif
            return false;
        }


        bool load_buffer_check_result(lua_State* l,int result)
        {
            if(result == 0)return true;
#if defined OOLUA_STORE_ERROR
            set_error_from_top_of_stack(l);
#elif defined OOLUA_EXCEPTIONS
            if(result == LUA_ERRFILE)
                throw OOLUA::File_error(m_lua);
            else if(result == LUA_ERRSYNTAX)
                throw OOLUA::Syntax_error(m_lua);
            else if(result == LUA_ERRMEM )
                throw OOLUA::Memory_error(m_lua);
#elif defined DEBUG || defined _DEBUG
            (void)l;
            if(result == LUA_ERRSYNTAX)
                assert(0 && "syntax error");
            else if(result == LUA_ERRMEM)
                assert(0 && "memory error");
            else if(result == LUA_ERRFILE)
                    assert(0 && "file error");
#endif
            return false;
        }
    }
}

