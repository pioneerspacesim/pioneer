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
	Faction *fac = LuaFaction::GetFromLua(1);
	lua_pushstring(l, fac->name.c_str());
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
	Faction *pFaction = LuaFaction::GetFromLua(1);
	lua_pushstring(l, pFaction->description_short.c_str());
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
	Faction *pFaction = LuaFaction::GetFromLua(1);
	lua_pushstring(l, pFaction->description.c_str());
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
	Faction *pFaction = LuaFaction::GetFromLua(1);
	lua_pushinteger(l, (pFaction->hasHomeworld) ? 1 : 0);
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
	Faction *pFaction = LuaFaction::GetFromLua(1);
	LuaSystemPath::PushToLua(&pFaction->homeworld);
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
	Faction *pFaction = LuaFaction::GetFromLua(1);
	lua_pushnumber(l, pFaction->foundingDate);
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
	Faction *pFaction = LuaFaction::GetFromLua(1);
	lua_pushnumber(l, pFaction->expansionRate);
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
	Faction *pFaction = LuaFaction::GetFromLua(1);
	lua_pushstring(l, pFaction->military_name.c_str());
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
	Faction *pFaction = LuaFaction::GetFromLua(1);
	lua_pushstring(l, pFaction->police_name.c_str());
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
	Faction *pFaction = LuaFaction::GetFromLua(1);
	lua_pushnumber(l, pFaction->colour.r);
	lua_pushnumber(l, pFaction->colour.g);
	lua_pushnumber(l, pFaction->colour.b);
	lua_pushnumber(l, pFaction->colour.a);
	return 4;
}

template <> const char *LuaObject< LuaUncopyable<Faction> >::s_type = "Faction";

template <> void LuaObject< LuaUncopyable<Faction> >::RegisterClass()
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
