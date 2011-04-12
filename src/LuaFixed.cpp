#include "LuaObject.h"
#include "LuaUtils.h"

template <> const char *LuaObject<LuaUncopyable<fixed> >::s_type = "Fixed";

template <> void LuaObject<LuaUncopyable<fixed> >::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ 0, 0 }
	};

	static const luaL_reg l_meta[] = {
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_meta);
}
