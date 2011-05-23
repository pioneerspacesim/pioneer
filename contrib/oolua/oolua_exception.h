///////////////////////////////////////////////////////////////////////////////
///  @file oolua_exception.h
///  @author Liam Devine
///  @email
///  See http://www.liamdevine.co.uk for contact details.
///  @licence
///  See licence.txt for more details.
///////////////////////////////////////////////////////////////////////////////
#ifndef OOLUA_EXCEPTION_H_
#	define OOLUA_EXCEPTION_H_


#include "oolua_config.h"

#if OOLUA_USE_EXCEPTIONS == 1

#	include "lua_includes.h"
#	include <stdexcept>
    namespace OOLUA
    {
		namespace ERROR 
		{
			struct PopTheStack{};
		}
        struct Bad_cast_error : public std::runtime_error
        {
            Bad_cast_error(std::string const & msg)
				:std::runtime_error(msg)
			{}
        };
        struct Syntax_error : public std::runtime_error
        {
            Syntax_error(lua_State* s)
                :std::runtime_error( lua_tostring(s, -1) )
            {}
			Syntax_error(lua_State* s,ERROR::PopTheStack*)
				:std::runtime_error( lua_tostring(s, -1) )
            {
				lua_pop(s,1);
			}
        };
        struct Runtime_error : public std::runtime_error
        {
            Runtime_error(lua_State* s)
                :std::runtime_error( lua_tostring(s, -1) )
            {}
			Runtime_error(lua_State* s,ERROR::PopTheStack*)
				:std::runtime_error( lua_tostring(s, -1) )
            {
				lua_pop(s,1);
			}
            Runtime_error(std::string const& str)
				:std::runtime_error( str )
            {}
        };
        struct Memory_error : public std::runtime_error
        {
            Memory_error(lua_State* s)
                :std::runtime_error( lua_tostring(s, -1) )
            {}
			Memory_error(lua_State* s,ERROR::PopTheStack*)
				:std::runtime_error( lua_tostring(s, -1) )
            {
				lua_pop(s,1);
			}
        };
        struct File_error : public std::runtime_error
        {
            File_error(lua_State* s)
                :std::runtime_error( lua_tostring(s, -1) )
            {}
			File_error(lua_State* s,ERROR::PopTheStack*)
				:std::runtime_error( lua_tostring(s, -1) )
            {
				lua_pop(s,1);
			}
        };
        struct Type_error : public std::runtime_error
        {
            Type_error(std::string const& str)
                :std::runtime_error( str )
            {}
        };
    }
# endif

#endif
