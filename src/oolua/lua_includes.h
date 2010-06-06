///////////////////////////////////////////////////////////////////////////////
///  @file lua_includes.h
///  Prevents name mangling and compatabilty for new Lua versions
///  @author Liam Devine
///////////////////////////////////////////////////////////////////////////////
#ifndef LUA_INCLUDES_H_
#	define LUA_INCLUDES_H_
//Prevent name mangling
extern "C"
{
#if defined _MSC_VER
#	include "lua/lua.h"
#	include "lua/lauxlib.h"
#	include "lua/lualib.h"
#elif defined __MINGW32__ //you may need to change this
#	include "lua/lua.h"
#	include "lua/lauxlib.h"
#	include "lua/lualib.h"
#elif defined __GNUC__
#	include "lua.h"
#	include "lauxlib.h"
#	include "lualib.h"
#endif

}


#if LUA_VERSION_NUM == 502 || LUA_VERSION_NUM > 502
//LUA_GLOBALSINDEX is deprecated and removed
//#	define LUA_GLOBALSINDEX  LUA_ENVIRONINDEX
#endif

#endif //LUA_INCLUDES_H_
