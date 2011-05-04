#include "LuaCargoBody.h"
#include "LuaUtils.h"
#include "LuaConstants.h"

static int l_cargobody_attr_type(lua_State *l)
{
	CargoBody *b = LuaCargoBody::GetFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", b->GetCargoType()));
	return 1;
}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<CargoBody*>(o);
}

template <> const char *LuaObject<CargoBody>::s_type = "CargoBody";

template <> void LuaObject<CargoBody>::RegisterClass()
{
	const char *l_parent = "Body";

	static const luaL_reg l_attrs[] = {
		{ "type", l_cargobody_attr_type },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, NULL, l_attrs, NULL);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
