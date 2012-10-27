// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaFaction.h"
#include "LuaSystemPath.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "Factions.h"

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
	const Faction *faction = LuaFaction::GetFromLua(1);
	lua_pushlstring(l, faction->name.c_str(), faction->name.size());
	return 1;
}

/*
 * Attribute: description_short
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
	const Faction *faction = LuaFaction::GetFromLua(1);
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
	const Faction *faction = LuaFaction::GetFromLua(1);
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
	const Faction *faction = LuaFaction::GetFromLua(1);
	lua_pushboolean(l, faction->hasHomeworld);
	return 1;
}

/*
 * Attribute: Homeworld
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
	Faction *faction = LuaFaction::GetFromLua(1);
	LuaSystemPath::PushToLua(&faction->homeworld);
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
	const Faction *faction = LuaFaction::GetFromLua(1);
	lua_pushnumber(l, faction->foundingDate);
	return 1;
}

/*
 * Attribute: expansionRate
 *
 * The rate at which the faction has been expanding since it's foundation.
 * Measured in light-years per-year of expansion. 
 * So for a value of 1.0 the volumes _RADIUS_ will expand by 1 light-year.
 * Used in conjunction with foundingDate it can be used to calculate the volume of occupied space.
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
	const Faction *faction = LuaFaction::GetFromLua(1);
	lua_pushnumber(l, faction->expansionRate);
	return 1;
}

/*
 * Attribute: military_name
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
	const Faction *faction = LuaFaction::GetFromLua(1);
	lua_pushlstring(l, faction->military_name.c_str(), faction->military_name.size());
	return 1;
}

/*
 * Attribute: police_name
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
static int l_faction_attr_police_name(lua_State *l)
{
	const Faction *faction = LuaFaction::GetFromLua(1);
	lua_pushlstring(l, faction->police_name.c_str(), faction->police_name.size());
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
	const Faction *faction = LuaFaction::GetFromLua(1);
	lua_createtable(l, 4, 0);
	lua_pushnumber(l, faction->colour.r);
	lua_rawseti(l, -2, 1);
	lua_pushnumber(l, faction->colour.g);
	lua_rawseti(l, -2, 2);
	lua_pushnumber(l, faction->colour.b);
	lua_rawseti(l, -2, 3);
	lua_pushnumber(l, faction->colour.a);
	lua_rawseti(l, -2, 4);
	return 1;
}

template <> const char *LuaObject<Faction>::s_type = "Faction";

template <> void LuaObject<Faction>::RegisterClass()
{
	static const luaL_Reg l_attrs[] = {
		{ "name",						l_faction_attr_name					},
		{ "description_short",			l_faction_attr_description_short    },
		{ "description",				l_faction_attr_description			},
		{ "has_homeworld",				l_faction_attr_has_homeworld		},
		{ "homeworld",					l_faction_attr_homeworld			},
		{ "founding_date",				l_faction_attr_founding_date		},
		{ "expansion_rate",				l_faction_attr_expansion_rate		},
		{ "military_name",				l_faction_attr_military_name		},
		{ "police_name",				l_faction_attr_police_name			},
		{ "colour",						l_faction_attr_colour				},
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, NULL, NULL, l_attrs, NULL);
}
