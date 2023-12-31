// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "galaxy/Factions.h"
#include "LuaConstants.h"
#include "LuaObject.h"
#include "LuaUtils.h"

/*
 * Class: Faction
 *
 * Class representing a single faction.
 *
 * <Faction> ... blah???
 */

/*
 * Attribute: name
 *
 * The name of the faction
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_name(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	lua_pushlstring(l, faction->name.c_str(), faction->name.size());
	return 1;
}

/*
 * Attribute: id
 *
 * A unique identification number of the faction
 *
 * Availability:
 *
 *  2014-06
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_id(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	lua_pushnumber(l, faction->idx);

	return 1;
}

/*
 * Attribute: descriptionShort
 *
 * The short description of the faction
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_description_short(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	lua_pushlstring(l, faction->description_short.c_str(), faction->description_short.size());
	return 1;
}

/*
 * Attribute: description
 *
 * The full length description of the faction
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_description(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	lua_pushlstring(l, faction->description.c_str(), faction->description.size());
	return 1;
}

/*
 * Attribute: hasHomeworld
 *
 * Does the faction have a homeworld?
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_has_homeworld(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	lua_pushboolean(l, faction->hasHomeworld);
	return 1;
}

/*
 * Attribute: homeworld
 *
 * Get the factions homeworld if it has one or a default SystemPath(0,0,0,0,0) if it doesn't
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_homeworld(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	LuaObject<SystemPath>::PushToLua(faction->homeworld);
	return 1;
}

/*
 * Attribute: foundingDate
 *
 * The date that the faction came into being.
 * Used in conjunction with expansionRate it can be used to calculate the volume of occupied space.
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_founding_date(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	lua_pushnumber(l, faction->foundingDate);
	return 1;
}

/*
 * Attribute: expansionRate
 *
 * The rate at which the faction has been expanding since it's foundation.
 * Measured in light-years per-year of expansion.
 * So for a value of 1.0 the volumes _RADIUS_ will expand by 1 light-year.
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_expansion_rate(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	lua_pushnumber(l, faction->expansionRate);
	return 1;
}

/*
 * Attribute: radius
 *
 * The radius in light years of the the spherical volume the faction
 * encompasses as at the year 3200
 *
 * Availability:
 *
 *  alpha 29
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_radius(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::GetFromLua(1);
	lua_pushnumber(l, faction->Radius());
	return 1;
}

/*
 * Attribute: militaryName
 *
 * The military name used by the faction
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_military_name(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	lua_pushlstring(l, faction->military_name.c_str(), faction->military_name.size());
	return 1;
}

/*
 * Attribute: policeName
 *
 * The name of the law enforcing agency in the faction
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_police_name(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	lua_pushlstring(l, faction->police_name.c_str(), faction->police_name.size());
	return 1;
}

/*
 * Attribute: policeShip
 *
 * The ships used by the police
 *
 * Availability:
 *
 *  2015 September
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_police_ship(lua_State *l)
{
	Faction *faction = LuaObject<Faction>::CheckFromLua(1);

	if (faction->police_ship.empty())
		faction->police_ship = "sinonatrix_police"; // set default ship

	lua_pushlstring(l, faction->police_ship.c_str(), faction->police_ship.size());
	return 1;
}

/*
 * Attribute: colour
 *
 * The colour used to represent the faction in the SectorView screen
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  experimental
 */
static int l_faction_attr_colour(lua_State *l)
{
	const Faction *faction = LuaObject<Faction>::CheckFromLua(1);
	lua_createtable(l, 0, 4);
	lua_pushnumber(l, faction->colour.r);
	lua_setfield(l, -2, "r");
	lua_pushnumber(l, faction->colour.g);
	lua_setfield(l, -2, "g");
	lua_pushnumber(l, faction->colour.b);
	lua_setfield(l, -2, "b");
	lua_pushnumber(l, faction->colour.a);
	lua_setfield(l, -2, "a");
	return 1;
}

template <>
const char *LuaObject<Faction>::s_type = "Faction";

template <>
void LuaObject<Faction>::RegisterClass()
{
	static const luaL_Reg l_attrs[] = {
		{ "name", l_faction_attr_name },
		{ "id", l_faction_attr_id },
		{ "descriptionShort", l_faction_attr_description_short },
		{ "description", l_faction_attr_description },
		{ "hasHomeworld", l_faction_attr_has_homeworld },
		{ "homeworld", l_faction_attr_homeworld },
		{ "foundingDate", l_faction_attr_founding_date },
		{ "expansionRate", l_faction_attr_expansion_rate },
		{ "radius", l_faction_attr_radius },
		{ "militaryName", l_faction_attr_military_name },
		{ "policeName", l_faction_attr_police_name },
		{ "policeShip", l_faction_attr_police_ship },
		{ "colour", l_faction_attr_colour },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, 0, l_attrs, 0);
}
