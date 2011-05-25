#ifndef OOLUA_CHAR_ARRAYS_H_
#	define OOLUA_CHAR_ARRAYS_H_
/*
This file is to do away with magic string literals
whilst also not paying the cost of a strlen
*/

//#include "lua_includes.h"

namespace OOLUA
{
	namespace INTERNAL
	{
		static const char change_mt_to_none_const_field[] = "__change_mt_to_none_const";
		static const char set_owner_str[] = "set_owner";
		static const char lua_owns_str[] = "Lua_owns";
		static const char cpp_owns_str[] = "Cpp_owns";
		static const char weak_lookup_name [] = "__weak_lookup";
		static const char new_str [] = "new";
	}
}

#define push_char_carray(lua,carray)lua_pushlstring(lua, carray, sizeof(carray)-1)

#endif
