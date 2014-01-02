// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "EnumStrings.h"
#include "Ship.h"
#include "Missile.h"
#include "LuaMissile.h"
#include "SpaceStation.h"
#include "ShipType.h"
#include "Sfx.h"
#include "Sound.h"
#include "Space.h"
#include "Pi.h"
#include "Player.h"
#include "HyperspaceCloud.h"
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

	Ship *s = LuaObject<Ship>::CheckFromLua(1);

	const char *type = luaL_checkstring(l, 2);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);

	s->SetShipType(type);
	s->m_equipment.Set(Equip::SLOT_ENGINE, 0, ShipType::types[type].hyperdrive);
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

	Ship *s = LuaObject<Ship>::CheckFromLua(1);

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

	Ship *s = LuaObject<Ship>::CheckFromLua(1);

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

	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:Explode() cannot be called on a ship in hyperspace");
	s->Explode();

	LUA_DEBUG_END(l, 0);
	return 0;
}

static int l_ship_get_skin(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaObject<SceneGraph::ModelSkin>::PushToLua(s->GetSkin());
	return 1;
}

static int l_ship_set_skin(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	const SceneGraph::ModelSkin *skin = LuaObject<SceneGraph::ModelSkin>::CheckFromLua(2);
	s->SetSkin(*skin);
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	const std::string label(luaL_checkstring(l, 2));
	s->SetLabel(label);
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstantFromArg(l, "EquipSlot", 2));
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
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
		lua_pushstring(l, EnumStrings::GetString("EquipType", e));
		return 1;
	} else {
		// 1-argument version; returns table of equipment items
		lua_newtable(l);

		for (int idx = 0; idx < size; idx++) {
			lua_pushinteger(l, idx+1);
			lua_pushstring(l, EnumStrings::GetString("EquipType", s->m_equipment.Get(slot, idx)));
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstantFromArg(l, "EquipType", 2));

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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstantFromArg(l, "EquipType", 2));

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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstantFromArg(l, "EquipSlot", 2));
	Equip::Type e = static_cast<Equip::Type>(LuaConstants::GetConstantFromArg(l, "EquipType", 3));
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	Equip::Slot slot = static_cast<Equip::Slot>(LuaConstants::GetConstantFromArg(l, "EquipSlot", 2));

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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	CargoBody * c_body = new CargoBody(static_cast<Equip::Type>(LuaConstants::GetConstantFromArg(l, "EquipType", 2)));
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetFlightState() != Ship::DOCKED) return 0;
	LuaObject<SpaceStation>::PushToLua(s->GetDockedWith());
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (!s->GetDockedWith())
		luaL_error(l, "Can't undock if not already docked");
	bool undocking = s->Undock();
	lua_pushboolean(l, undocking);
	return 1;
}

/* Method: SpawnMissile
 *
 * Spawn a missile near the ship.
 *
 * > missile = ship:SpawnMissile(type, target, power)
 * 
 * Parameters:
 *
 *   shiptype - a string for the missile type. specifying an
 *          ship that is not a missile will result in a Lua error
 *
 *   target - the <Ship> to fire the missile at
 *
 *   power - the power of the missile. If unspecified, the default power for the
 *
 * Return:
 *
 *   missile - The missile spawned, or nil if it was unsuccessful.
 *
 * Availability:
 *
 *   alpha 26
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_spawn_missile(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:SpawnMissile() cannot be called on a ship in hyperspace");
	ShipType::Id missile_type(luaL_checkstring(l, 2));

	if (missile_type != ShipType::MISSILE_UNGUIDED &&
			missile_type != ShipType::MISSILE_GUIDED &&
			missile_type != ShipType::MISSILE_SMART &&
			missile_type != ShipType::MISSILE_NAVAL)
		luaL_error(l, "Ship type '%s' is not a valid missile type", lua_tostring(l, 2));
	int power = (lua_isnone(l, 3))? -1 : lua_tointeger(l, 3);

	Missile * missile = s->SpawnMissile(missile_type, power);
	if (missile)
		LuaObject<Missile>::PushToLua(missile);
	else
		lua_pushnil(l);
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	SystemPath *dest = LuaObject<SystemPath>::CheckFromLua(2);

	int fuel;
	double duration;
	Ship::HyperjumpStatus status = s->CheckHyperspaceTo(*dest, fuel, duration);

	lua_pushstring(l, EnumStrings::GetString("ShipJumpStatus", status));
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	SystemPath *dest = LuaObject<SystemPath>::CheckFromLua(2);

	int fuel;
	double duration;
	Ship::HyperjumpStatus status = s->GetHyperspaceDetails(*dest, fuel, duration);

	lua_pushstring(l, EnumStrings::GetString("ShipJumpStatus", status));
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	SystemPath *dest = LuaObject<SystemPath>::CheckFromLua(2);

	int fuel;
	double duration;
	Ship::HyperjumpStatus status = s->CheckHyperspaceTo(*dest, fuel, duration);

	lua_pushstring(l, EnumStrings::GetString("ShipJumpStatus", status));
	if (status != Ship::HYPERJUMP_OK)
		return 1;

	s->StartHyperspaceCountdown(*dest);
	lua_pushinteger(l, fuel);
	lua_pushnumber(l, duration);
	return 3;
}

/*
 * Method: GetInvulnerable
 *
 * Find out whether a ship can take damage or not.
 *
 * > is_invulnerable = ship:GetInvulnerable()
 *
 * Return:
 *
 *   is_invulnerable - boolean; true if the ship is invulnerable to damage
 *
 * Availability:
 *
 *  November 2013
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_get_invulnerable(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	lua_pushboolean(l, s->IsInvulnerable());
	return 1;
}

/*
 * Method: SetInvulnerable
 *
 * Make a ship invulnerable to damage (or not).
 * Note: Invulnerability is not currently stored in the save game.
 *
 * > ship:SetInvulnerable(true)
 *
 * Availability:
 *
 *  November 2013
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_set_invulnerable(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	luaL_checkany(l, 2);
	s->SetInvulnerable(lua_toboolean(l, 2));
	return 0;
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIKill() cannot be called on a ship in hyperspace");
	Ship *target = LuaObject<Ship>::CheckFromLua(2);
	s->AIKill(target);
	return 0;
}

/*
 * Method: AIKamikaze
 *
 * Crash into the target ship.
 *
 * > ship:AIKamikaze(target)
 *
 * Parameters:
 *
 *   target - the <Ship> to destroy
 *
 * Availability:
 *
 *  alpha 26
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_ai_kamikaze(lua_State *l)
{
	Ship *s = LuaObject<Ship>::GetFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIKamikaze() cannot be called on a ship in hyperspace");
	Ship *target = LuaObject<Ship>::GetFromLua(2);
	s->AIKamikaze(target);
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIFlyTo() cannot be called on a ship in hyperspace");
	Body *target = LuaObject<Body>::CheckFromLua(2);
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIDockWith() cannot be called on a ship in hyperspace");
	SpaceStation *target = LuaObject<SpaceStation>::CheckFromLua(2);
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIEnterLowOrbit() cannot be called on a ship in hyperspace");
	Body *target = LuaObject<Body>::CheckFromLua(2);
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIEnterMediumOrbit() cannot be called on a ship in hyperspace");
	Body *target = LuaObject<Body>::CheckFromLua(2);
	if (!target->IsType(Object::PLANET) && !target->IsType(Object::STAR))
		luaL_argerror(l, 2, "expected a Planet or a Star");
	s->AIOrbit(target, 1.6);
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:AIEnterHighOrbit() cannot be called on a ship in hyperspace");
	Body *target = LuaObject<Body>::CheckFromLua(2);
	if (!target->IsType(Object::PLANET) && !target->IsType(Object::STAR))
		luaL_argerror(l, 2, "expected a Planet or a Star");
	s->AIOrbit(target, 3.2);
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
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	s->AIClearInstructions();
	return 0;
}

template <> const char *LuaObject<Ship>::s_type = "Ship";

template <> void LuaObject<Ship>::RegisterClass()
{
	static const char *l_parent = "Body";

	static const luaL_Reg l_methods[] = {
		{ "IsPlayer", l_ship_is_player },

		{ "SetShipType", l_ship_set_type },
		{ "SetHullPercent", l_ship_set_hull_percent },
		{ "SetFuelPercent", l_ship_set_fuel_percent },

		{ "GetSkin",    l_ship_get_skin    },
		{ "SetSkin",    l_ship_set_skin    },
		{ "SetLabel",   l_ship_set_label   },

		{ "GetEquipSlotCapacity", l_ship_get_equip_slot_capacity },
		{ "GetEquip",         l_ship_get_equip           },
		{ "SetEquip",         l_ship_set_equip           },
		{ "AddEquip",         l_ship_add_equip           },
		{ "RemoveEquip",      l_ship_remove_equip        },
		{ "GetEquipCount",    l_ship_get_equip_count     },
		{ "GetEquipFree",     l_ship_get_equip_free      },

		{ "SpawnCargo", l_ship_spawn_cargo },

		{ "SpawnMissile", l_ship_spawn_missile },

		{ "GetDockedWith", l_ship_get_docked_with },
		{ "Undock",        l_ship_undock          },

		{ "Explode", l_ship_explode },

		{ "AIKill",             l_ship_ai_kill               },
		{ "AIKamikaze",         l_ship_ai_kamikaze           },
		{ "AIFlyTo",            l_ship_ai_fly_to             },
		{ "AIDockWith",         l_ship_ai_dock_with          },
		{ "AIEnterLowOrbit",    l_ship_ai_enter_low_orbit    },
		{ "AIEnterMediumOrbit", l_ship_ai_enter_medium_orbit },
		{ "AIEnterHighOrbit",   l_ship_ai_enter_high_orbit   },
		{ "CancelAI",           l_ship_cancel_ai             },

		{ "CheckHyperspaceTo", l_ship_check_hyperspace_to },
		{ "GetHyperspaceDetails", l_ship_get_hyperspace_details },
		{ "HyperspaceTo",    l_ship_hyperspace_to     },

		{ "GetInvulnerable", l_ship_get_invulnerable },
		{ "SetInvulnerable", l_ship_set_invulnerable },

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Ship>::DynamicCastPromotionTest);
}

/*
 * Group: Attributes
 *
 *
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
 *
 *
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
 *
 *
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
 *
 *
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
 *
 *
 * Attribute: fuelMassLeft
 *
 * Remaining thruster fuel mass in tonnes.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 *
 *
 * Attribute: hullMassLeft
 *
 * Remaining hull integrity in tonnes. Ship damage reduces hull integrity.
 * When this reaches 0, the ship is destroyed.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 *
 *
 * Attribute: shieldMassLeft
 *
 * Remaining shield strength in tonnes. As shields are depleted, the
 * shield strength decreases. When this reaches 0, the shields are
 * fully depleted and the hull is exposed to damage.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 *
 *
 * Attribute: shieldMass
 *
 * Maximum shield strength for installed shields. Measured in tonnes.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 *
 *
 * Attribute: hyperspaceRange
 *
 * Furthest possible hyperjump given current hyperspace fuel available.
 * Measured in light-years.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 *
 *
 * Attribute: maxHyperspaceRange
 *
 * Furthest possible hyperjump assuming no limits to available hyperspace fuel.
 * Measured in light-years.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 *
 *
 * Attribute: totalMass
 *
 * Mass of the ship including hull, equipment and cargo, but excluding
 * thruster fuel mass. Measured in tonnes.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 *
 *
 * Attribute: usedCapacity
 *
 * Hull capacity used by equipment and cargo. Measured in tonnes.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 *
 *
 * Attribute: usedCargo
 *
 * Hull capacity used by cargo only (not equipment). Measured in tonnes.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 *
 *
 * Attribute: freeCapacity
 *
 * Total space remaining. Measured in tonnes.
 *
 * Availability:
 *
 *   November 2013
 *
 * Status:
 *
 *   experimental
 *
 */
