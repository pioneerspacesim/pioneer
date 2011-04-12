#include "LuaObject.h"
#include "LuaUtils.h"

template <> const char *LuaObject<Planet>::s_type = "Planet";

template <> void LuaObject<Planet>::RegisterClass()
{
	LuaObjectBase::CreateClass(s_type, NULL, NULL, NULL);
}
