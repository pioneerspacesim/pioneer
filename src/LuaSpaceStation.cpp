// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaUtils.h"
#include "SpaceStation.h"
#include "LuaChatForm.h"
#include "LuaConstants.h"

/*
 * Class: SpaceStation
 *
 * Class representing a space station. Inherits from <Body>
 */

/*
 * Method: GetEquipmentPrice
 *
 * Get the price of an equipment or cargo item traded at this station
 *
 * > price = station:GetEquipmentPrice(equip)
 *
 * Parameters:
 *
 *   equip - the <Constants.EquipType> string for the equipment or cargo item
 *
 * Returns:
 *
 *   price - the price of the equipment or cargo item
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_spacestation_get_equipment_price(lua_State *l)
{
	SpaceStation *s = LuaObject<SpaceStation>::CheckFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstantFromArg(l, "EquipType", 2));
	Sint64 cost = s->GetPrice(e);
	lua_pushnumber(l, cost * 0.01);
	return 1;
}

/*
 * Method: GetEquipmentStock
 *
 * Get the quantity of an equipment or cargo item this station has available for trade
 *
 * > stock = station:GetEquipmentStock(equip)
 *
 * Parameters:
 *
 *   equip - the <Constants.EquipType> string for the equipment or cargo item
 *
 * Returns:
 *
 *   stock - the amount available for trade
 *
 * Availability:
 *
 *   201308
 *
 * Status:
 *
 *   experimental
 */
static int l_spacestation_get_equipment_stock(lua_State *l)
{
	SpaceStation *s = LuaObject<SpaceStation>::CheckFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstantFromArg(l, "EquipType", 2));
	int stock = s->GetStock(e);
	lua_pushinteger(l, stock);
	return 1;
}

/*
 * Method: AddEquipmentStock
 *
 * Modify the quantity of an equipment or cargo item this station has available for trade
 *
 * > station:AddEquipmentStock(equip, amount)
 *
 * Parameters:
 *
 *   equip - the <Constants.EquipType> string for the equipment or cargo item
 *
 *   amount - the amount of the item to add (or substract) from the station stock
 *
 * Availability:
 *
 *   201308
 *
 * Status:
 *
 *   experimental
 */
static int l_spacestation_add_equipment_stock(lua_State *l)
{
	SpaceStation *s = LuaObject<SpaceStation>::CheckFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstantFromArg(l, "EquipType", 2));
	int stock = luaL_checkinteger(l, 3);
	s->AddEquipmentStock(e, stock);
	return 0;
}

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
	const char *l_parent = "Body";

	static const luaL_Reg l_methods[] = {
		{ "GetEquipmentPrice", l_spacestation_get_equipment_price },
		{ "GetEquipmentStock", l_spacestation_get_equipment_stock },
		{ "AddEquipmentStock", l_spacestation_add_equipment_stock },

		{ "GetGroundPosition",  l_spacestation_get_ground_position },

		{ 0, 0 }
	};

	static luaL_Reg l_attrs[] = {
		{ "numDocks",        l_spacestation_attr_num_docks         },
		{ "isGroundStation", l_spacestation_attr_is_ground_station },

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<SpaceStation>::DynamicCastPromotionTest);
}
