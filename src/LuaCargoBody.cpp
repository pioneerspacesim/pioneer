// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaCargoBody.h"
#include "LuaUtils.h"
#include "LuaConstants.h"

/*
 * Class: CargoBody
 *
 * Class representing an item of cargo floating in space. Inherits from
 * <Body>.
 */

/*
 * Attribute: type
 *
 * The type of cargo contained within this cargo body, as a
 * <Constants.EquipType> constant.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_cargobody_attr_type(lua_State *l)
{
	CargoBody *b = LuaCargoBody::CheckFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", b->GetCargoType()));
	return 1;
}

template <> const char *LuaObject<CargoBody>::s_type = "CargoBody";

template <> void LuaObject<CargoBody>::RegisterClass()
{
	const char *l_parent = "Body";

	static const luaL_Reg l_attrs[] = {
		{ "type", l_cargobody_attr_type },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, NULL, l_attrs, NULL);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<CargoBody>::DynamicCastPromotionTest);
}
