#include "LuaCargoBody.h"
#include "LuaUtils.h"

static int l_cargobody_get_cargo_type(lua_State *l)
{
	CargoBody *b = LuaCargoBody::GetFromLua(1);
	lua_pushinteger(l, b->GetCargoType());
	return 1;
}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<CargoBody*>(o);
}

template <> const char *LuaObject<CargoBody>::s_type = "CargoBody";

template <> void LuaObject<CargoBody>::RegisterClass()
{
	const char *l_inherit = "Body";

	static const luaL_reg l_methods[] = {
		{ "GetCargoType", l_cargobody_get_cargo_type },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_inherit, l_methods, NULL, NULL);
	LuaObjectBase::RegisterPromotion(l_inherit, s_type, promotion_test);
}
