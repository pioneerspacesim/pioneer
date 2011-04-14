#include "LuaObject.h"
#include "LuaShipType.h"
#include "LuaUtils.h"
#include "ShipType.h"

template <> const char *LuaObject<LuaUncopyable<ShipType> >::s_type = "ShipType";

template <> void LuaObject<LuaUncopyable<ShipType> >::RegisterClass()
{
	static const luaL_reg l_methods[] = {
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, NULL);
}
