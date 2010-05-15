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


#ifdef OOLUA_EXCEPTIONS
#	if defined __GNUC__ && !defined _EXCEPTIONS
#			error OOLua has been compiled with exceptions yet they have been disabled for this build 
#	elif defined _MSC_VER && !defined _HAS_EXCEPTIONS
#			error OOLua has been compiled with exceptions yet they have been disabled for this build
#	endif
#endif


# ifdef OOLUA_NO_EXCEPTIONS

#	include "lua_includes.h"
    namespace OOLUA
    {
        struct Bad_cast_error : public std::runtime_error
        {
            Bad_cast_error(std::string const & msg):std::runtime_error(msg){}
        };
        struct Syntax_error : public std::runtime_error
        {
            Syntax_error(lua_State* s)
                :std::runtime_error( lua_tostring(s, -1) )
            {}
        };
        struct Runtime_error : public std::runtime_error
        {
            Runtime_error(lua_State* s)
                :std::runtime_error( lua_tostring(s, -1) )
            {}
        };
        struct Memory_error : public std::runtime_error
        {
            Memory_error(lua_State* s)
                :std::runtime_error( lua_tostring(s, -1) )
            {}
        };
        struct File_error : public std::runtime_error
        {
            File_error(lua_State* s)
                :std::runtime_error( lua_tostring(s, -1) )
            {}
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
