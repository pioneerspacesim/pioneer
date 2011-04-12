#include "LuaObject.h"
#include "LuaUtils.h"

template <> const char *LuaObject<LuaUncopyable<vector3f> >::s_type = "Vector";

template <> void LuaObject<LuaUncopyable<vector3f> >::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ 0, 0 }
	};

	static const luaL_reg l_meta[] = {
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_meta);
}
