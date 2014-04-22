// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaUtils.h"
#include "SpaceStation.h"
#include "LuaConstants.h"

/*
 * Class: SpaceStation
 *
 * Class representing a space station. Inherits from <ModelBody>
 */

/*
 * Method: GetGroundPosition
 *
 * Get latitude, longitude of a station on the ground or nil if this is an orbital station
 *
 * > latitude, longitude = station:GetGroundPosition()
 *
 * Returns:
 *
 *   latitude - the latitude of the station in radians
 *   longitude - the longitude of the station in radians
 *
 * Examples:
 *
 * > -- Get ground position of L.A. when starting on Earth
 * > local la = Game.player:GetDockedWith()
 * > local lat, long = la:GetGroundPosition()
 * > lat = math.rad2deg(lat)
 * > long = math.rad2deg(long)
 *
 * Availability:
 *
 *   July 2013
 *
 * Status:
 *
 *   experimental
 */
static int l_spacestation_get_ground_position(lua_State *l)
{
	SpaceStation *s = LuaObject<SpaceStation>::CheckFromLua(1);
	if (!s->IsGroundStation())
		return 0;

	vector3d pos = s->GetPosition();
	double latitude = atan2(pos.y, sqrt(pos.x*pos.x + pos.z * pos.z));
	double longitude = atan2(pos.x, pos.z);
	lua_pushnumber(l, latitude);
	lua_pushnumber(l, longitude);
	return 2;
}

/*
 * Attribute: numDocks
 *
 * The number of docking ports a spacestation has.
 *
 * Availability:
 *
 *   alpha 21
 *
 * Status:
 *
 *   experimental
 */
static int l_spacestation_attr_num_docks(lua_State *l)
{
	SpaceStation *s = LuaObject<SpaceStation>::CheckFromLua(1);
	lua_pushinteger(l, s->GetDockingPortCount());
	return 1;
}

/*
 * Attribute: numShipsDocked
 *
 * The number of ships docked at spacestation
 *
 * Availability:
 *
 *   201404
 *
 * Status:
 *
 *   experimental
 */
static int l_spacestation_attr_num_ships_docked(lua_State *l)
{
	SpaceStation *s = LuaObject<SpaceStation>::CheckFromLua(1);
	lua_pushinteger(l, s->NumShipsDocked());
	return 1;
}

/*
 * Attribute: isGroundStation
 *
 * true if station is on the ground, false if its an orbital
 *
 * Availability:
 *
 *   alpha 30
 *
 * Status:
 *
 *   experimental
 */
static int l_spacestation_attr_is_ground_station(lua_State *l)
{
	SpaceStation *s = LuaObject<SpaceStation>::CheckFromLua(1);
	lua_pushboolean(l, s->IsGroundStation());
	return 1;
}

template <> const char *LuaObject<SpaceStation>::s_type = "SpaceStation";

template <> void LuaObject<SpaceStation>::RegisterClass()
{
	const char *l_parent = "ModelBody";

	static const luaL_Reg l_methods[] = {
		{ "GetGroundPosition",  l_spacestation_get_ground_position },

		{ 0, 0 }
	};

	static luaL_Reg l_attrs[] = {
		{ "numDocks",        l_spacestation_attr_num_docks         },
		{ "isGroundStation", l_spacestation_attr_is_ground_station },
		{ "numShipsDocked",  l_spacestation_attr_num_ships_docked  },

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<SpaceStation>::DynamicCastPromotionTest);
}
