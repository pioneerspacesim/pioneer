// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaShip.h"
#include "LuaSpaceStation.h"
#include "LuaSystemPath.h"
#include "LuaShipType.h"
#include "LuaBody.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "ShipType.h"
#include "Sfx.h"
#include "Sound.h"
#include "Space.h"
#include "Pi.h"
#include "Player.h"
#include "HyperspaceCloud.h"
#include "LmrModel.h"
#include "Game.h"

/*
 * Class: Ship
 *
 * Class representing a ship. Inherits from <Body>.
 */

/*
 * Group: Methods
 */

/*
 * Method: IsPlayer
 *
 * Determines if the ship is the player ship
 *
 * > isplayer = ship:IsPlayer()
 *
 * Returns:
 *
 *   isplayer - true if the ship is the player, false otherwise
 *
 * Example:
 *
 * > if Game.player:IsPlayer() then
 * >     print("this is the player")
 * > end
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_ship_is_player(lua_State *l)
{
    lua_pushboolean(l, false);
    return 1;
}

/*
 * Method: GetStats
 *
 * Returns statistics for the ship
 *
 * > stats = ship:GetStats()
 *
 * Returns:
 *
 *   stats - a table with the following fields
 *
 *     maxCapacity - maximum space for cargo and equipment (t)
 *     usedCapacity - amount of space used (t)
 *     usedCargo - amount of cargo space used (t)
 *     freeCapacity - total space remaining (t)
 *     totalMass - total mass of the ship (cargo, equipment & hull) (t)
 *     hullMassLeft - remaining hull mass. when this reaches 0, the ship is destroyed (t)
 *     shieldMass - total mass equivalent of all shields (t)
 *     shieldMassLeft - remaining shield mass. when this reaches 0, the shields are depleted and the hull is exposed (t)
 *     hyperspaceRange - distance of furthest possible jump based on current contents (ly)
 *     maxHyperspaceRange - distance furthest possible jump under ideal conditions (ly)
 *     maxFuelTankMass - mass of internal (thruster) fuel tank, when full (t)
 *     fuelMassLeft - current mass of the internal fuel tank (t)
 *     fuelUse - thruster fuel use, scaled for the strongest thrusters at full thrust (percentage points per second, e.g. 0.0003)
 *
 * Example:
 *
 * > local stats = ship:GetStats()
 * > if stats.shieldMass == stats.shieldMassLeft then
 * >     print("shields at full strength")
 * > end
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_get_stats(lua_State *l)
{
	LUA_DEBUG_START(l);

	Ship *s = LuaShip::CheckFromLua(1);
	const shipstats_t &stats = s->GetStats();

	lua_newtable(l);
	pi_lua_settable(l, "maxCapacity",        stats.max_capacity);
	pi_lua_settable(l, "usedCapacity",       stats.used_capacity);
	pi_lua_settable(l, "usedCargo",          stats.used_cargo);
	pi_lua_settable(l, "freeCapacity",       stats.free_capacity);
	pi_lua_settable(l, "totalMass",          stats.total_mass);
	pi_lua_settable(l, "hullMassLeft",       stats.hull_mass_left);
	pi_lua_settable(l, "hyperspaceRange",    stats.hyperspace_range);
	pi_lua_settable(l, "maxHyperspaceRange", stats.hyperspace_range_max);
	pi_lua_settable(l, "shieldMass",         stats.shield_mass);
	pi_lua_settable(l, "shieldMassLeft",     stats.shield_mass_left);
	pi_lua_settable(l, "maxFuelTankMass",    stats.fuel_tank_mass);
	pi_lua_settable(l, "fuelUse",            stats.fuel_use);
	pi_lua_settable(l, "fuelMassLeft",       stats.fuel_tank_mass_left);

	LUA_DEBUG_END(l, 1);

	return 1;
}

/* Method: SetShipType
 *
 * Replaces the ship with a new ship of the specified type.
 *
 * > ship:SetShipType(newtype)
 *
 * Parameters:
 *
 *   newtype - the name of the ship
 *
 * Example:
 *
 * > ship:SetShipType('sirius_interdictor')
 *
 * Availability:
 *
 *   alpha 15
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_set_type(lua_State *l)
{
	LUA_DEBUG_START(l);

	Ship *s = LuaShip::CheckFromLua(1);

	const char *type = luaL_checkstring(l, 2);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);

	ShipFlavour f(type);

	s->ResetFlavour(&f);
	s->m_equipment.Set(Equip::SLOT_ENGINE, 0, ShipType::types[f.id].hyperdrive);
	s->UpdateStats();

	LUA_DEBUG_END(l, 0);

	return 0;
}

/*
 * Method: SetHullPercent
 *
 * Sets the hull mass of the ship to the given precentage of its maximum.
 *
 * > ship:SetHullPercent(percent)
 *
 * Setting the hull percentage to 0 will not destroy the ship until it takes
 * damage.
 *
 * Parameters:
 *
 *   percent - optional. A number from 0 to 100. Less then 0 will use 0 and
 *             greater than 100 will use 100. Defaults to 100.
 *
 * Example:
 *
 * > ship:SetHullPercent(3.14)
 *
 * Availability:
 *
 *  alpha 15
 *
 * Status:
 *
 *  experimental
 */

static int l_ship_set_hull_percent(lua_State *l)
{
	LUA_DEBUG_START(l);

	Ship *s = LuaShip::CheckFromLua(1);

	float percent = 100;
	if (lua_isnumber(l, 2)) {
		percent = float(luaL_checknumber(l, 2));
		if (percent < 0.0f || percent > 100.0f) {
			pi_lua_warn(l,
				"argument out of range: Ship{%s}:SetHullPercent(%g)",
				s->GetLabel().c_str(), percent);
		}
	}

	s->SetPercentHull(percent);

	LUA_DEBUG_END(l, 0);

	return 0;
}

/*
 * Method: SetFuelPercent
 *
 * Sets the thruster fuel tank of the ship to the given precentage of its maximum.
 *
 * > ship:SetFuelPercent(percent)
 *
 * Parameters:
 *
 *   percent - optional. A number from 0 to 100. Less then 0 will use 0 and
 *             greater than 100 will use 100. Defaults to 100.
 *
 * Example:
 *
 * > ship:SetFuelPercent(50)
 *
 * Availability:
 *
 *  alpha 20
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_set_fuel_percent(lua_State *l)
{
	LUA_DEBUG_START(l);

	Ship *s = LuaShip::CheckFromLua(1);

	float percent = 100;
	if (lua_isnumber(l, 2)) {
		percent = float(luaL_checknumber(l, 2));
		if (percent < 0.0f || percent > 100.0f) {
			pi_lua_warn(l,
				"argument out of range: Ship{%s}:SetFuelPercent(%g)",
				s->GetLabel().c_str(), percent);
		}
	}

	s->SetFuel(percent/100.f);

	LUA_DEBUG_END(l, 0);

	return 0;
}

/*
 * Method: Explode
 *
 * Destroys the ship in an explosion
 *
 * > ship:Explode()
 *
 * Availability:
 *
 * 	alpha 20
 *
 * Status:
 *
 * 	experimental
 */

static int l_ship_explode(lua_State *l)
{
	LUA_DEBUG_START(l);

	Ship *s = LuaShip::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:Explode() cannot be called on a ship in hyperspace");
	s->Explode();

	LUA_DEBUG_END(l, 0);
	return 0;
}

/*
 * Method: SetLabel
 *
 * Changes the ship's label text. This is the text that appears beside the
 * ship in the HUD.
 *
 * > ship:SetLabel(newlabel)
 *
 * Parameters:
 *
 *   newlabel - the new label
 *
 * Example:
 *
 * > ship:SetLabel("AB-1234")
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_ship_set_label(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	const char *label = luaL_checkstring(l, 2);

	ShipFlavour f = *(s->GetFlavour());
	f.regid = label;
	s->UpdateFlavour(&f);

	s->SetLabel(label);
	return 0;
}

static void _prepare_colour(lua_State *l, LmrMaterial &m)
{
	float r, g, b;

	if (lua_type(l, 2) == LUA_TSTRING) {
		const char *hexstr = lua_tostring(l, 2);
		if (strlen(hexstr) != 7 || *hexstr != '#')
			luaL_error(l, "Colour string should be a hex triple (#rrggbb)");
		int n = strtol(&hexstr[1], NULL, 16);
		r = float((n & 0xff0000) >> 16);
		g = float((n & 0xff00) >> 8);
		b = float(n & 0xff);
	}

	else {
		r = luaL_checknumber(l, 2);
		g = luaL_checknumber(l, 3);
		b = luaL_checknumber(l, 4);
	}

	r /= 256;
	g /= 256;
	b /= 256;

	float invmax = 1.0f / std::max(r, std::max(g, b));

	r *= invmax;
	g *= invmax;
	b *= invmax;

	m.diffuse[0] = 0.5f * r;
	m.diffuse[1] = 0.5f * g;
	m.diffuse[2] = 0.5f * b;
	m.diffuse[3] = 1.0f;
	m.specular[0] = r;
	m.specular[1] = g;
	m.specular[2] = b;
	m.specular[3] = 0.0f;
	m.emissive[0] = m.emissive[1] = m.emissive[2] = m.emissive[3] = 0.0f;
	m.shininess = 50.0f + float(Pi::rng.Double())*50.0f;
}

/*
 * Method: SetPrimaryColour
 *
 * Change the ship model's primary colour
 *
 * > ship:SetPrimaryColour(hex)
 * > ship:SetPrimaryColour(red, green, blue)
 *
 * Parameters:
 *
 *   hex - a hex RGB triplet describing the colour, eg. "#99cc99"
 *
 *   red - a real number describing the red component of the colour. 0.0 is no
 *         red component, 1.0 is full red
 *
 *   green - a real number describing the green component of the colour. 0.0
 *           is no green component, 1.0 is full green
 *
 *   blue - a real number describing the blue component of the colour. 0.0 is
 *          no blue component, 1.0 is full blue
 *
 * Example:
 *
 * > ship:SetPrimaryColour("#002366")       -- royal blue
 * > ship:SetPrimaryColour(1.0, 0.27, 0.0)  -- orange red
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  deprecated
 */
static int l_ship_set_primary_colour(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);

	ShipFlavour f = *(s->GetFlavour());
	_prepare_colour(l, f.primaryColor);
	s->UpdateFlavour(&f);

	return 0;
}

/*
 * Method: SetSecondaryColour
 *
 * Change the ship model's secondary colour
 *
 * > ship:SetSecondaryColour(hex)
 * > ship:SetSecondaryColour(red, green, blue)
 *
 * Parameters:
 *
 *   hex - a hex RGB triplet describing the colour, eg. "#99cc99"
 *
 *   red - a real number describing the red component of the colour. 0.0 is no
 *         red component, 1.0 is full red
 *
 *   green - a real number describing the green component of the colour. 0.0
 *           is no green component, 1.0 is full green
 *
 *   blue - a real number describing the blue component of the colour. 0.0 is
 *          no blue component, 1.0 is full blue
 *
 * Example:
 *
 * > ship:SetSecondaryColour("#002366")       -- royal blue
 * > ship:SetSecondaryColour(1.0, 0.27, 0.0)  -- orange red
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  deprecated
 */
static int l_ship_set_secondary_colour(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);

	ShipFlavour f = *(s->GetFlavour());
	_prepare_colour(l, f.secondaryColor);
	s->UpdateFlavour(&f);

	return 0;
}

static inline void _colour_to_table(lua_State *l, const char *name, const float rgba[4])
{
	lua_newtable(l);
	pi_lua_settable(l, "r", rgba[0]);
	pi_lua_settable(l, "g", rgba[1]);
	pi_lua_settable(l, "b", rgba[2]);
	pi_lua_settable(l, "a", rgba[3]);
	lua_setfield(l, -2, name);
}

/*
 * Method: SetFlavour
 *
 * Set various attributes that describe variations in the way the ship model
 * is rendered.
 *
 * > ship:SetFlavour(flavour)
 *
 * The recommended way to use this method is to get the existing flavour from
 * the ship's <flavour> attribute, modify the data you want, and then call
 * <SetFlavour> to reset it.
 *
 * Parameters:
 *
 *   flavour - a table with structure as defined in <flavour>.
 *
 * Availability:
 *
 *   alpha 27
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_set_flavour(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	ShipFlavour f = ShipFlavour::FromLuaTable(l, 2);
	s->UpdateFlavour(&f);
	return 0;
}

/*
 * Method: GetEquipSlotCapacity
 *
 * Get the maximum number of a particular type of equipment this ship can
 * hold. This is the number of items that can be held, not the mass.
 * <AddEquip> will take care of ensuring the hull capacity is not exceeded.
 *
 * > capacity = shiptype:GetEquipSlotCapacity(slot)
 *
 * Parameters:
 *
 *   slot - a <Constants.EquipSlot> string for the wanted equipment type
 *
 * Returns:
 *
 *   capacity - the maximum capacity of the equipment slot
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_get_equip_slot_capacity(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 2)));
	lua_pushinteger(l, s->m_equipment.GetSlotSize(slot));
	return 1;
}

/*
 * Method: GetEquip
 *
 * Get a list of equipment in a given equipment slot
 *
 * > equip = ship:GetEquip(slot, index)
 * > equiplist = ship:GetEquip(slot)
 *
 * Parameters:
 *
 *   slot - a <Constants.EquipSlot> string for the wanted equipment type
 *
 *   index - optional. The equipment position in the slot to fetch. If
 *           specified the item at that position in the slot will be returned,
 *           otherwise a table containing all items in the slot will be
 *           returned instead.
 *
 * Return:
 *
 *   equip - when index is specified, a <Constants.EquipType> string for the
 *           item
 *
 *   equiplist - when index is not specified, a table of zero or more
 *               <Constants.EquipType> strings for all the items in the slot
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_get_equip(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	const char *slotName = luaL_checkstring(l, 2);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", slotName));

	int size = s->m_equipment.GetSlotSize(slot);

	if (lua_isnumber(l, 3)) {
		// 2-argument version; returns the item in the specified slot index
		int idx = lua_tointeger(l, 3) - 1;
		if (idx >= size || idx < 0) {
			pi_lua_warn(l,
				"argument out of range: Ship{%s}:GetEquip('%s', %d)",
				s->GetLabel().c_str(), slotName, idx+1);
		}
		Equip::Type e = (idx >= 0) ? s->m_equipment.Get(slot, idx) : Equip::NONE;
		lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", e));
		return 1;
	} else {
		// 1-argument version; returns table of equipment items
		lua_newtable(l);

		for (int idx = 0; idx < size; idx++) {
			lua_pushinteger(l, idx+1);
			lua_pushstring(l, LuaConstants::GetConstantString(l, "EquipType", s->m_equipment.Get(slot, idx)));
			lua_rawset(l, -3);
		}

		return 1;
	}
}

/*
 * Method: SetEquip
 *
 * Overwrite a single item of equipment in a given equipment slot
 *
 * > ship:SetEquip(slot, index, equip)
 *
 * Parameters:
 *
 *   slot - a <Constants.EquipSlot> string for the equipment slot
 *
 *   index - the position to store the item in
 *
 *   equip - a <Constants.EquipType> string for the item
 *
 * Example:
 *
 * > -- add a laser to the rear laser mount
 * > ship:SetEquip("LASER", 1, "PULSECANNON_1MW")
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_set_equip(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	const char *slotName = luaL_checkstring(l, 2);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", slotName));
	int idx = luaL_checkinteger(l, 3) - 1;
	const char *typeName = luaL_checkstring(l, 4);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", typeName));

	// XXX should go through a Ship::SetEquip() wrapper method that checks mass constraints

	if (idx < 0 || idx >= s->m_equipment.GetSlotSize(slot)) {
		pi_lua_warn(l,
			"argument out of range: Ship{%s}:SetEquip('%s', %d, '%s')",
			s->GetLabel().c_str(), slotName, idx+1, typeName);
		return 0;
	}

	s->m_equipment.Set(slot, idx, e);
	s->UpdateEquipStats();
	return 0;
}

/*
 * Method: AddEquip
 *
 * Add an equipment or cargo item to its appropriate equipment slot
 *
 * > num_added = ship:AddEquip(item, count)
 *
 * Parameters:
 *
 *   item - a <Constants.EquipType> string for the item
 *
 *   count - optional. The number of this item to add. Defaults to 1.
 *
 * Return:
 *
 *   num_added - the number of items added. Can be less than count if there
 *               was not enough room.
 *
 * Example:
 *
 * > ship:AddEquip("ANIMAL_MEAT", 10)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_add_equip(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 2)));

	int num = luaL_optinteger(l, 3, 1);
	if (num < 0)
		return luaL_error(l, "Can't add a negative number of equipment items.");

	const shipstats_t &stats = s->GetStats();
	if (Equip::types[e].mass != 0)
		num = std::min(stats.free_capacity / (Equip::types[e].mass), num);

	lua_pushinteger(l, s->m_equipment.Add(e, num));
	s->UpdateEquipStats();
	return 1;
}

/*
 * Method: RemoveEquip
 *
 * Remove one or more of a given equipment type from its appropriate cargo slot
 *
 * > num_removed = ship:RemoveEquip(item, count)
 *
 * Parameters:
 *
 *   item - a <Constants.EquipType> string for the item
 *
 *   count - optional. The number of this item to remove. Defaults to 1.
 *
 * Return:
 *
 *   num_removed - the number of items removed
 *
 * Example:
 *
 * > ship:RemoveEquip("DRIVE_CLASS1")
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_remove_equip(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 2)));

	int num = luaL_optinteger(l, 3, 1);
	if (num < 0)
		return luaL_error(l, "Can't remove a negative number of equipment items.");

	lua_pushinteger(l, s->m_equipment.Remove(e, num));
	s->UpdateEquipStats();
	return 1;
}

/*
 * Method: GetEquipCount
 *
 * Get the number of a given equipment or cargo item in a given equipment slot
 *
 * > count = ship:GetEquipCount(slot, item)
 *
 * Parameters:
 *
 *   slot - a <Constants.EquipSlot> string for the slot
 *
 *   item - a <Constants.EquipType> string for the item
 *
 * Return:
 *
 *   count - the number of the given item in the slot
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_get_equip_count(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 2)));
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 3)));
	lua_pushinteger(l, s->m_equipment.Count(slot, e));
	return 1;
}

/*
 * Method: GetEquipFree
 *
 * Get the amount of free space in a given equipment slot
 *
 * > free = ship:GetEquipFree(slot)
 *
 * Parameters:
 *
 *   slot - a <Constants.EquipSlot> string for the slot to check
 *
 * Return:
 *
 *   free - the number of item spaces left in this slot
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_get_equip_free(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstant(l, "EquipSlot", luaL_checkstring(l, 2)));

	lua_pushinteger(l, s->m_equipment.FreeSpace(slot));
	return 1;
}

/*
 * Method: SpawnCargo
 *
 * Spawns a container right next to the ship.
 *
 * > success = ship:SpawnCargo(item)
 *
 * Parameters:
 *
 *  item - the item to put in the container.
 *
 * Result:
 *
 *   success: true if the container was spawned, false otherwise.
 *
 * Availability:
 *
 *   alpha 26
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_spawn_cargo(lua_State *l) {
	Ship *s = LuaShip::CheckFromLua(1);
	CargoBody * c_body = new CargoBody(static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 2))));
    lua_pushboolean(l, s->SpawnCargo(c_body));
    return 1;
}

/*
 * Method: GetDockedWith
 *
 * Get the station that the ship is currently docked with
 *
 * > station = ship:GetDockedWith()
 *
 * Return:
 *
 *   station - a <SpaceStation> object for the station, or nil if the ship is
 *             not docked
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_ship_get_docked_with(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	if (s->GetFlightState() != Ship::DOCKED) return 0;
	LuaSpaceStation::PushToLua(s->GetDockedWith());
	return 1;
}

/*
 * Method: Undock
 *
 * Undock from the station currently docked with
 *
 * > success = ship:Undock()
 *
 * <Event.onShipUndocked> will be triggered once undocking is complete
 *
 * Return:
 *
 *   success - true if ship is undocking, false if the ship is unable to undock,
 *             probably because another ship is currently undocking
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  stable
 */
static int l_ship_undock(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	if (!s->GetDockedWith())
		luaL_error(l, "Can't undock if not already docked");
	bool undocking = s->Undock();
	lua_pushboolean(l, undocking);
	return 1;
}

/*
 * Method: FireMissileAt
 *
 * Fire a missile at the given target
 *
 * > fired = ship:FireMissileAt(type, target)
 *
 * Parameters:
 *
 *   type - a <Constants.EquipType> string for the missile type. specifying an
 *          equipment that is not a missile will result in a Lua error
 *
 *   target - the <Ship> to fire the missile at
 *
 * Return:
 *
 *   fired - true if the missile was fired, false if the ship has no missile
 *           of the requested type
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_fire_missile_at(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:FireMissileAt() cannot be called on a ship in hyperspace");
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstant(l, "EquipType", luaL_checkstring(l, 2)));
	Ship *target = LuaShip::CheckFromLua(3);

	if (e < Equip::MISSILE_UNGUIDED || e > Equip::MISSILE_NAVAL)
		luaL_error(l, "Equipment type '%s' is not a valid missile type", lua_tostring(l, 2));

	int max_missiles = s->m_equipment.GetSlotSize(Equip::SLOT_MISSILE);
	int idx;
	for (idx = 0; idx < max_missiles; idx++)
		if (s->m_equipment.Get(Equip::SLOT_MISSILE, idx) == e)
			break;

	if (idx == max_missiles) {
		lua_pushboolean(l, false);
		return 1;
	}

	lua_pushboolean(l, s->FireMissile(idx, target));
	return 1;
}

/*
 * Method: CheckHyperspaceTo
 *
 * Determine is a ship is able to hyperspace to a given system
 *
 * > status, fuel, duration = ship:CheckHyperspaceTo(path)
 *
 * The result is based on distance, range, available fuel, ship mass and other
 * factors. If this returns a status of 'OK' then the jump is valid right now.
 *
 * Parameters:
 *
 *   path - a <SystemPath> for the destination system
 *
 * Result:
 *
 *   status - a <Constants.ShipJumpStatus> string that tells if the ship can
 *            hyperspace and if not, describes the reason
 *
 *   fuel - if status is 'OK', contains the amount of fuel required to make
 *          the jump (tonnes)
 *
 *   duration - if status is 'OK', contains the time that the jump will take
 *				(seconds)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_ship_check_hyperspace_to(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	SystemPath *dest = LuaSystemPath::CheckFromLua(2);

	int fuel;
	double duration;
	Ship::HyperjumpStatus status = s->CheckHyperspaceTo(*dest, fuel, duration);

	lua_pushstring(l, LuaConstants::GetConstantString(l, "ShipJumpStatus", status));
	if (status == Ship::HYPERJUMP_OK) {
		lua_pushinteger(l, fuel);
		lua_pushnumber(l, duration);
		return 3;
	}
	return 1;
}

/*
 * Method: GetHyperspaceDetails
 *
 * Compute the fuel requirement and duration of a jump.
 *
 * > status, fuel, duration = ship:GetHyperspaceDetails(path)
 *
 * The result is based on distance, range, available fuel, ship mass and other
 * factors. It does not check flight state (that is, it can return 'OK' even if
 * the ship is docked, docking or landed, in which case an actual jump would fail).
 *
 * Parameters:
 *
 *   path - a <SystemPath> for the destination system
 *
 * Result:
 *
 *   status - a <Constants.ShipJumpStatus> string that tells if the ship can
 *            hyperspace and if not, describes the reason
 *
 *   fuel - if status is 'OK', contains the amount of fuel required to make
 *          the jump (tonnes)
 *
 *   duration - if status is 'OK', contains the time that the jump will take
 *				(seconds)
 *
 * Availability:
 *
 *   alpha 21
 *
 * Status:
 *
 *   stable
 */
static int l_ship_get_hyperspace_details(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	SystemPath *dest = LuaSystemPath::CheckFromLua(2);

	int fuel;
	double duration;
	Ship::HyperjumpStatus status = s->GetHyperspaceDetails(*dest, fuel, duration);

	lua_pushstring(l, LuaConstants::GetConstantString(l, "ShipJumpStatus", status));
	if (status == Ship::HYPERJUMP_OK) {
		lua_pushinteger(l, fuel);
		lua_pushnumber(l, duration);
		return 3;
	}
	return 1;
}

/*
 * Method: HyperspaceTo
 *
 * Initiate hyperspace jump to a given system
 *
 * > status = ship:HyperspaceTo(path)
 *
 * If the status returned is "OK", then a hyperspace departure cloud will be
 * created where the ship was and the <Event.onLeaveSystem> event will be
 * triggered.
 *
 * Parameters:
 *
 *   path - a <SystemPath> for the destination system
 *
 * Result:
 *
 *   status - a <Constants.ShipJumpStatus> string for the result of the jump
 *            attempt
 *
 *   fuel - if status is 'OK', contains the amount of fuel required to make
 *          the jump (tonnes)
 *
 *   duration - if status is 'OK', contains the time that the jump will take
 *              (seconds)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_ship_hyperspace_to(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	SystemPath *dest = LuaSystemPath::CheckFromLua(2);

	int fuel;
	double duration;
	Ship::HyperjumpStatus status = s->CheckHyperspaceTo(*dest, fuel, duration);

	lua_pushstring(l, LuaConstants::GetConstantString(l, "ShipJumpStatus", status));
	if (status != Ship::HYPERJUMP_OK)
		return 1;

	s->StartHyperspaceCountdown(*dest);
	lua_pushinteger(l, fuel);
	lua_pushnumber(l, duration);
	return 3;
}

/*
 * Group: Attributes
 */

/*
 * Attribute: flavour
 *
 * Various attributes that describe variations in the way the ship model is
 * rendered.
 *
 * flavour is a table with the following keys:
 *
 *   id - the id (name) of the ship definition
 *
 *   regId - the registration ID that will be displayed on the side of the
 *           ship. Usually the same as the ship's label
 *
 *   price - trade price for the ship
 *
 *   primaryColour - a table describing the ship's primary colour. Contains
 *                  three tables "diffuse", "specular" and "emissive", each
 *                  containing values "r", "g", "b" and "a" for the colour,
 *                  and a fourth value "shininess". These form a typical
 *                  OpenGL material
 *
 *   secondaryColour - a table describing the ship's secondary colour. The
 *                     structure is the same as primaryColour
 *
 * Availability:
 *
 *   alpha 27
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_attr_flavour(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);

	ShipFlavour f = *(s->GetFlavour());

	lua_newtable(l);
	pi_lua_settable(l, "id",    f.id.c_str());
	pi_lua_settable(l, "regId", f.regid.c_str());
	pi_lua_settable(l, "price", double(f.price)*0.01);

	lua_newtable(l);
	_colour_to_table(l, "diffuse", f.primaryColor.diffuse);
	_colour_to_table(l, "specular", f.primaryColor.specular);
	_colour_to_table(l, "emissive", f.primaryColor.emissive);
	pi_lua_settable(l, "shininess", f.primaryColor.shininess);
	lua_setfield(l, -2, "primaryColour");

	lua_newtable(l);
	_colour_to_table(l, "diffuse", f.secondaryColor.diffuse);
	_colour_to_table(l, "specular", f.secondaryColor.specular);
	_colour_to_table(l, "emissive", f.secondaryColor.emissive);
	pi_lua_settable(l, "shininess", f.secondaryColor.shininess);
	lua_setfield(l, -2, "secondaryColour");

	return 1;
}

/*
 * Attribute: alertStatus
 *
 * The current alert status of the ship. A <Constants.ShipAlertStatus> string.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_attr_alert_status(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "ShipAlertStatus", s->GetAlertState()));
	return 1;
}

/*
 * Attribute: flightState
 *
 * The current flight state of the ship. A <Constants.ShipFlightState> string.
 *
 * Availability:
 *
 *  alpha 25
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_attr_flight_state(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	lua_pushstring(l, LuaConstants::GetConstantString(l, "ShipFlightState", s->GetFlightState()));
	return 1;
}

/*
 * Attribute: shipId
 *
 * The internal id of the ship type. This value can be passed to
 * <ShipType.GetShipType> to retrieve information about this ship type.
 *
 * Availability:
 *
 *  alpha 28
 *
 * Status:
 *
 *  stable
 */
static int l_ship_attr_ship_id(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	const ShipType &st = s->GetShipType();
	lua_pushstring(l, st.id.c_str());
	return 1;
}

/*
 * Attribute: shipType
 *
 * The internal id of the ship type. This value can be passed to
 * <ShipType.GetShipType> to retrieve information about this ship type.
 *
 * This attribute is deprecated. Use <shipId> instead.
 *
 * Availability:
 *
 *  alpha 19
 *
 * Status:
 *
 *  deprecated
 */
static int l_ship_attr_ship_type(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	const ShipType &st = s->GetShipType();
	lua_pushstring(l, st.id.c_str());
	return 1;
}

/*
 * Attribute: fuel
 *
 * The current amount of fuel, as a percentage of full
 *
 * Availability:
 *
 *   alpha 20
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_attr_fuel(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	lua_pushnumber(l, s->GetFuel() * 100.f);
	return 1;
}


/*
 * Group: AI methods
 *
 * The AI methods are the script's equivalent of the autopilot. They are
 * high-level commands to instruct the ship to fly somewhere and possibly take
 * some action when it arrives (like dock or attack).
 *
 * When an AI completes the <Event.onAICompleted> event is triggered, and
 * the ship is left with engines off in whatever state the AI left it in. For
 * some AI methods (eg <AIEnterLowOrbit>) this is useful. For others it will
 * likely mean the ship will eventually succumb to gravity and crash
 * somewhere. You should invoke another AI method or take some other action to
 * prevent this.
 */

/*
 * Method: AIKill
 *
 * Attack a target ship and continue until it is destroyed
 *
 * > ship:AIKill(target)
 *
 * Note the combat AI currently will fly the ship and fire the lasers as
 * necessary, but it will not activate any other equipment (missiles, ECM,
 * etc). It is the responsbility of the script to take those additional
 * actions if desired.
 *
 * Parameters:
 *
 *   target - the <Ship> to destroy
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_ai_kill(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIKill() cannot be called on a ship in hyperspace");
	Ship *target = LuaShip::CheckFromLua(2);
	s->AIKill(target);
	return 0;
}

/*
 * Method: AIFlyTo
 *
 * Fly to the vicinity of a given physics body
 *
 * > ship:AIFlyTo(target)
 *
 * Parameters:
 *
 *   target - the <Body> to fly to
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_ai_fly_to(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIFlyTo() cannot be called on a ship in hyperspace");
	Body *target = LuaBody::CheckFromLua(2);
	s->AIFlyTo(target);
	return 0;
}

/*
 * Method: AIDockWith
 *
 * Fly to and dock with a given station
 *
 * > ship:AIDockWith(target)
 *
 * Parameters:
 *
 *   target - the <SpaceStation> to dock with
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_ai_dock_with(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIDockWith() cannot be called on a ship in hyperspace");
	SpaceStation *target = LuaSpaceStation::CheckFromLua(2);
	s->AIDock(target);
	return 0;
}

/*
 * Method: AIEnterLowOrbit
 *
 * Fly to and enter a low orbit around a given planet or star
 *
 * > ship:AIEnterLowOrbit(target)
 *
 * Parameters:
 *
 *   target - the <Star> or <Planet> to orbit
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_ai_enter_low_orbit(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIEnterLowOrbit() cannot be called on a ship in hyperspace");
	Body *target = LuaBody::CheckFromLua(2);
	if (!target->IsType(Object::PLANET) && !target->IsType(Object::STAR))
		luaL_argerror(l, 2, "expected a Planet or a Star");
	s->AIOrbit(target, 1.2);
	return 0;
}

/*
 * Method: AIEnterMediumOrbit
 *
 * Fly to and enter a medium orbit around a given planet or star
 *
 * > ship:AIEnterMediumOrbit(target)
 *
 * Parameters:
 *
 *   target - the <Star> or <Planet> to orbit
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_ai_enter_medium_orbit(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIEnterMediumOrbit() cannot be called on a ship in hyperspace");
	Body *target = LuaBody::CheckFromLua(2);
	if (!target->IsType(Object::PLANET) && !target->IsType(Object::STAR))
		luaL_argerror(l, 2, "expected a Planet or a Star");
	s->AIOrbit(target, 2.0);
	return 0;
}

/*
 * Method: AIEnterHighOrbit
 *
 * Fly to and enter a high orbit around a given planet or star
 *
 * > ship:AIEnterHighOrbit(target)
 *
 * Parameters:
 *
 *   target - the <Star> or <Planet> to orbit
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_ai_enter_high_orbit(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIEnterHighOrbit() cannot be called on a ship in hyperspace");
	Body *target = LuaBody::CheckFromLua(2);
	if (!target->IsType(Object::PLANET) && !target->IsType(Object::STAR))
		luaL_argerror(l, 2, "expected a Planet or a Star");
	s->AIOrbit(target, 5.0);
	return 0;
}

/*
 * Method: CancelAI
 *
 * Cancel the current AI command
 *
 * > ship:CancelAI()
 *
 * This ship is left with the orientation and velocity it had when <CancelAI>
 * was called. The engines are switched off.
 *
 * Note that <Event.onAICompleted> will not be triggered by calling
 * <CancelAI>, as the AI did not actually complete.
 *
 * You do not need to call this if you intend to immediately invoke another AI
 * method. Calling an AI method will replace the previous command if one
 * exists.
 *
 * Availability:
 *
 *  alpha 10
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_cancel_ai(lua_State *l)
{
	Ship *s = LuaShip::CheckFromLua(1);
	s->AIClearInstructions();
	return 0;
}

template <> const char *LuaObject<Ship>::s_type = "Ship";

template <> void LuaObject<Ship>::RegisterClass()
{
	static const char *l_parent = "Body";

	static const luaL_Reg l_methods[] = {
		{ "IsPlayer", l_ship_is_player },

		{ "GetStats", l_ship_get_stats },
		{ "SetShipType", l_ship_set_type },
		{ "SetHullPercent", l_ship_set_hull_percent },
		{ "SetFuelPercent", l_ship_set_fuel_percent },

		{ "SetLabel",           l_ship_set_label            },
		{ "SetPrimaryColour",   l_ship_set_primary_colour   },
		{ "SetSecondaryColour", l_ship_set_secondary_colour },
		{ "SetFlavour",         l_ship_set_flavour          },

		{ "GetEquipSlotCapacity", l_ship_get_equip_slot_capacity },
		{ "GetEquip",         l_ship_get_equip           },
		{ "SetEquip",         l_ship_set_equip           },
		{ "AddEquip",         l_ship_add_equip           },
		{ "RemoveEquip",      l_ship_remove_equip        },
		{ "GetEquipCount",    l_ship_get_equip_count     },
		{ "GetEquipFree",     l_ship_get_equip_free      },

		{ "SpawnCargo", l_ship_spawn_cargo },

		{ "FireMissileAt", l_ship_fire_missile_at },

		{ "GetDockedWith", l_ship_get_docked_with },
		{ "Undock",        l_ship_undock          },

		{ "Explode", l_ship_explode },

		{ "AIKill",             l_ship_ai_kill               },
		{ "AIFlyTo",            l_ship_ai_fly_to             },
		{ "AIDockWith",         l_ship_ai_dock_with          },
		{ "AIEnterLowOrbit",    l_ship_ai_enter_low_orbit    },
		{ "AIEnterMediumOrbit", l_ship_ai_enter_medium_orbit },
		{ "AIEnterHighOrbit",   l_ship_ai_enter_high_orbit   },
		{ "CancelAI",           l_ship_cancel_ai             },

		{ "CheckHyperspaceTo", l_ship_check_hyperspace_to },
		{ "GetHyperspaceDetails", l_ship_get_hyperspace_details },
		{ "HyperspaceTo",    l_ship_hyperspace_to     },

		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
        { "flavour",     l_ship_attr_flavour },
		{ "alertStatus", l_ship_attr_alert_status },
		{ "flightState", l_ship_attr_flight_state },
		{ "shipId",      l_ship_attr_ship_id },
		{ "shipType",    l_ship_attr_ship_type },
		{ "fuel",        l_ship_attr_fuel },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, NULL);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Ship>::DynamicCastPromotionTest);
}
