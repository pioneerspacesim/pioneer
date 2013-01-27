// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaMissile.h"

/*
 * Class: Missile
 *
 * Class representing a missile. Inherits from <Ship>.
 */

/*
 * Group: Methods
 */

/*
 * Method: IsArmed
 *
 * Determines if the missile is armed.
 *
 * > isarmed = missile:IsArmed()
 *
 * Returns:
 *
 *   isarmed - true if the missile is armed, false otherwise.
 *
 * Example:
 *
 * > if approaching_missile:IsArmed() then
 * >     print("DANGER! DANGER!")
 * > end
 *
 * Availability:
 *
 *  alpha 31
 *
 * Status:
 *
 *  experimental
 */
static int l_missile_is_armed(lua_State *l)
{
	Missile * m = LuaMissile::CheckFromLua(1);
	lua_pushboolean(l, m->IsArmed());
	return 1;
}

/*
 * Method: Arm
 *
 * Arms the missile
 *
 * > missile:Arm()
 *
 * Availability:
 *
 *  alpha 31
 *
 * Status:
 *
 *  experimental
 */
static int l_missile_arm(lua_State *l)
{
	Missile * m = LuaMissile::CheckFromLua(1);
	m->Arm();
	return 0;
}

/*
 * Method: Disarm
 *
 * Disarms the missile
 *
 * > missile:Disarm()
 *
 * Availability:
 *
 *  alpha 31
 *
 * Status:
 *
 *  experimental
 */
static int l_missile_disarm(lua_State *l)
{
	Missile * m = LuaMissile::CheckFromLua(1);
	m->Disarm();
	return 0;
}

template <> const char *LuaObject<Missile>::s_type = "Missile";

template <> void LuaObject<Missile>::RegisterClass()
{
	static const char *l_parent = "Ship";

	static const luaL_Reg l_methods[] = {
		{ "IsArmed", l_missile_is_armed },
		{ "Arm",     l_missile_arm },
		{ "Disarm",  l_missile_disarm },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = { { 0, 0} };

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, NULL);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Missile>::DynamicCastPromotionTest);
}
