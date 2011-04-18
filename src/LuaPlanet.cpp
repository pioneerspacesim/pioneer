#include "LuaPlanet.h"
#include "LuaUtils.h"

template <> const char *LuaObject<Planet>::s_type = "Planet";

template <> void LuaObject<Planet>::RegisterClass()
{
	const char *l_inherit = "Body";

	LuaObjectBase::CreateClass(s_type, l_inherit, NULL, NULL);
}
