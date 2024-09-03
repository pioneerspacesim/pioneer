// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaMissile.h"

#include "Ship.h"
#include "ShipAICmd.h"

/*
 * Class: Missile
 *
 * Class representing a missile. Inherits from <Ship>.
 */

/*
 * Group: Methods
 */

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
	Missile *m = LuaMissile::CheckFromLua(1);
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
	Missile *m = LuaMissile::CheckFromLua(1);
	m->Disarm();
	return 0;
}

/*
 * Method: AIKamikaze
 *
 * Crash into the target ship.
 *
 * > missile:AIKamikaze(target)
 *
 * Parameters:
 *
 *   target - the <Ship> to destroy
 *
 * Returns:
 *   true if the command could be enacted, false otherwise
 *
 * Availability:
 *
 *  Gen 2017
 *
 * Status:
 *
 *  experimental
 */
static int l_missile_ai_kamikaze(lua_State *l)
{
	Missile *m = LuaObject<Missile>::GetFromLua(1);
	Ship *target = LuaObject<Ship>::GetFromLua(2);
	if (target != nullptr) {
		m->AIKamikaze(target);
		lua_pushboolean(l, true);
	} else {
		lua_pushboolean(l, false);
	}
	return 1;
}

/*
 * Group: Attributes
 */

/*
 * Attribute: isArmed
 *
 * Determines if the missile is armed. True is farmed, false otherwise.
 *
 * Example:
 *
 * > if approaching_missile:isArmed then
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
static int l_missile_attr_is_armed(lua_State *l)
{
	Missile *m = LuaMissile::CheckFromLua(1);
	lua_pushboolean(l, m->IsArmed());
	return 1;
}

template <>
const char *LuaObject<Missile>::s_type = "Missile";

template <>
void LuaObject<Missile>::RegisterClass()
{
	static const char *l_parent = "ModelBody"; // "DynamicBody";

	static const luaL_Reg l_methods[] = {
		{ "Arm", l_missile_arm },
		{ "Disarm", l_missile_disarm },
		{ "AIKamikaze", l_missile_ai_kamikaze },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "isArmed", l_missile_attr_is_armed },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Missile>::DynamicCastPromotionTest);
}
