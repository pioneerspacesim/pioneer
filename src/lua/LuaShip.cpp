// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CargoBody.h"
#include "EnumStrings.h"
#include "Frame.h"
#include "Game.h"
#include "HyperspaceCloud.h"
#include "Lua.h"
#include "LuaObject.h"
#include "LuaTable.h"
#include "LuaVector.h"
#include "Missile.h"
#include "Pi.h"
#include "Ship.h"
#include "ShipAICmd.h"
#include "ShipType.h"
#include "Space.h"
#include "SpaceStation.h"
#include "ship/PlayerShipController.h"
#include "ship/PrecalcPath.h"
#include "lua.h"

/*
 * Class: Ship
 *
 * Class representing a ship. Inherits from <ModelBody>.
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
	if (!ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);

	s->SetShipType(type);
	s->UpdateEquipStats();

	LUA_DEBUG_END(l, 0);

	return 0;
}

/* Method: GetShipType
 *
 * Returns a string describing the ship type
 *
 * > local shiptype = ship:GetShipType()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_ship_get_ship_type(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaPush(l, s->GetShipType()->name.c_str());
	return 1;
}

/* Method: GetShipClass
 *
 * Returns a string describing the ship class
 *
 * > local shipclass = ship:GetShipClass()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_ship_get_ship_class(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaPush(l, s->GetShipType()->shipClass.c_str());
	return 1;
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

	double percent = 100;
	if (lua_isnumber(l, 2)) {
		percent = luaL_checknumber(l, 2);
		if (percent < 0.0 || percent > 100.0) {
			pi_lua_warn(l,
				"argument out of range: Ship{%s}:SetFuelPercent(%g)",
				s->GetLabel().c_str(), percent);
		}
	}

	s->SetFuel(percent / 100.0);
	// This is required for the fuel to properly update when the game is paused.
	// Without this we can sell fuel indefinitely, or buy fuel without receiving
	// anything. This is a workaround - the proper fix is to prevent interactions
	// with stations while the game is paused.
	s->TimeStepUpdate(0.0);

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

/*
 * Method: GetSkin
 *
 * Get the current skin object of the ship.
 *
 * > ship:GetSkin()
 *
 * Parameters:
 *
 *
 *
 * Example:
 *
 * > ship:GetSkin()
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_get_skin(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaObject<SceneGraph::ModelSkin>::PushToLua(s->GetSkin());
	return 1;
}

/*
 * Method: SetSkin
 *
 * Set the skin of the ship.
 *
 * > ship:SetSkin(skin)
 *
 * Parameters:
 *
 *   skin - the skin object of the ship
 *   this can be created using SceneGraph.ModelSkin.New()
 *
 * Example:
 *
 * > ship:GetSkin(skin)
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_set_skin(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	const SceneGraph::ModelSkin *skin = LuaObject<SceneGraph::ModelSkin>::CheckFromLua(2);
	s->SetSkin(*skin);
	return 0;
}

/*
 * Method: SetPattern
 *
 * Changes the pattern used for texturing the ship.
 *
 * > ship:SetPattern(num)
 *
 * Parameters:
 *
 *   num - the pattern number
 *
 * Example:
 *
 * > ship:SetPattern(5)
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_set_pattern(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	unsigned int num = lua_tointeger(l, 2);
	SceneGraph::Model *model = s->GetModel();
	if (model && model->SupportsPatterns()) {
		if (num > model->GetNumPatterns() - 1)
			return luaL_error(l, "This pattern does not exist for this ship");

		s->SetPattern(num);
	}
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
 * Method: SetShipName
 *
 * Changes the ship's name text.
 * This is the name text that appears beside the ship in the HUD.
 *
 * > ship:SetShipName(newShipName)
 *
 * Parameters:
 *
 *   newShipName - the new name of the ship
 *
 * Example:
 *
 * > ship:SetShipName("Boris")
 *
 * Availability:
 *
 *  September 2014
 *
 * Status:
 *
 *  stable
 */
static int l_ship_set_ship_name(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	const std::string shipname(luaL_checkstring(l, 2));
	s->SetShipName(shipname);
	return 0;
}

/*
 * Method: SpawnCargo
 *
 * Spawns a container right next to the ship.
 *
 * > success = ship:SpawnCargo(item, lifeTime)
 *
 * Parameters:
 *
 *  item - the item to put in the container.
 *
 *  lifeTime - optional. time in seconds until self destruct. Setting
 *             to 0 sec means no self destruct while player is in system.
 *
 * Result:
 *
 *   success: true if the container was spawned, false otherwise.
 *
 * Example:
 *
 * > Game.player:SpawnCargo(Commodities.hydrogen, 0)
 * > Game.player:SpawnCargo(Commodities.hydrogen)
 *
 * Availability:
 *
 *   alpha 26
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_spawn_cargo(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (!lua_istable(l, 2)) {
		lua_pushboolean(l, false);
	}

	CargoBody *c_body;
	const char *model;

	lua_getfield(l, 2, "model_name");
	if (lua_isstring(l, -1))
		model = lua_tostring(l, -1);
	else
		model = "cargo";

	if (lua_gettop(l) >= 3) {
		float lifeTime = lua_tonumber(l, 3);
		c_body = new CargoBody(model, LuaRef(l, 2), lifeTime);
	} else
		c_body = new CargoBody(model, LuaRef(l, 2));

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
 * Method: SetDockedWith
 *
 * Set the station as docked with the given space station. May not work properly if the ship is already docked.
 *
 * > success = ship:SetDockedWith(station)
 *
 * Parameters:
 *
 *   station - a SpaceStation to dock the current ship with
 *
 * Return:
 *
 *   success - a boolean indicating whether the current ship was able to be docked at the station
 *
 * Availability:
 *
 *   2022-11
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_set_docked_with(lua_State *l)
{
	Ship *ship = LuaObject<Ship>::CheckFromLua(1);
	SpaceStation *station = LuaObject<SpaceStation>::CheckFromLua(2);

	int port = station->GetFreeDockingPort(ship); // pass in the ship to get a port we fit into
	if (port < 0) {
		lua_pushboolean(l, false);
		return 0;
	}

	ship->SetFrame(station->GetFrame());
	ship->SetDockedWith(station, port);
	lua_pushboolean(l, true);
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

/* Method: BlastOff
 *
 * Blast off if landed on a surface.
 *
 */
static int l_ship_blast_off(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (!s->IsLanded())
		luaL_error(l, "Can't blast off if not already landed");
	s->Blastoff();
	return 0;
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
	int power = (lua_isnone(l, 3)) ? -1 : lua_tointeger(l, 3);

	Missile *missile = s->SpawnMissile(missile_type, power);
	if (missile)
		LuaObject<Missile>::PushToLua(missile);
	else
		lua_pushnil(l);
	return 1;
}

/* Method: UseECM
 *
 * Activates ECM of ship, destroying nearby missile with probability
 * proportional to proximity.
 *
 * > success, recharge_wait = Ship:UseECM()
 *
 * Return:
 *
 *   success - is true or false depending on if the ECM was activated
 *             or not. False indicating wither it is not fully
 *             recharged, or there is no ECM to activate.
 *
 *   recharge_wait - time left to recharge.
 *
 * Availability:
 *
 *   2014 July
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_use_ecm(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetFlightState() == Ship::HYPERSPACE)
		return luaL_error(l, "Ship:UseECM() cannot be called on a ship in hyperspace");

	Ship::ECMResult result = s->UseECM();

	float recharge;

	if (result == Ship::ECMResult::ECM_ACTIVATED) {
		recharge = s->GetECMRechargeRemain();
		lua_pushboolean(l, true);
		lua_pushnumber(l, recharge);
	} else if (result == Ship::ECMResult::ECM_RECHARGING) {
		recharge = s->GetECMRechargeRemain();
		lua_pushboolean(l, false);
		lua_pushnumber(l, recharge);
	} else if (result == Ship::ECMResult::ECM_NOT_INSTALLED) {
		lua_pushboolean(l, false);
		lua_pushnil(l);
	}

	return 2;
}

static int l_ship_is_ecm_ready(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaPush<bool>(l, is_equal_exact(s->GetECMRechargeRemain(), 0.0));
	return 1;
}

/*
 * Method: GetDurationForDistance
 *
 *   Calculating the duration of the flight of a given ship at a specified distance.
 *   Assumed that at the beginning and at the end of the travel the speed is 0.
 *
 * > duration = ship:GetDurationForDistance(distance)
 *
 * Parameters:
 *
 *   distance - length, in meters
 *
 * Result:
 *
 *   duration - travel time, in seconds.
 *
 */
static int l_ship_get_duration_for_distance(lua_State *l)
{
	Ship *ship = LuaObject<Ship>::CheckFromLua(1);
	double distance = LuaPull<double>(l, 2);
	const ShipType *st = ship->GetShipType();
	const shipstats_t ss = ship->GetStats();
	PrecalcPath pp(
		distance, // distance
		0.0,	  // velocity at start
		st->effectiveExhaustVelocity,
		st->linThrust[THRUSTER_FORWARD],
		st->linAccelerationCap[THRUSTER_FORWARD],
		1000 * (ss.static_mass + ss.fuel_tank_mass_left), // 100% mass of the ship
		1000 * ss.fuel_tank_mass_left * 0.8,			  // multipied to 0.8 have fuel reserve
		0.85);											  // braking margin
	LuaPush<double>(l, pp.getFullTime());
	return 1;
}

/*
 * Method: InitiateHyperjumpTo
 *
 *   Ready the ship to jump to the given system. This does not perform
 *   any check regarding hyperdrive class, range, fuel. Nor does it
 *   respect minimum legal distance for hyperjump. For those features use
 *   <Ship.HyperjumpTo> instead.
 *
 * > status = ship:InitiateHyperjumpTo(path, warmup, duration, sounds, checks)
 *
 * Parameters:
 *
 *   path - a <SystemPath> for the destination system
 *
 *   warmup - the time, in seconds, needed for the engines to warm up.
 *            Minimum time is one second, for safety reasons.
 *
 *   duration - travel time, in seconds.
 *
 *   sounds - a table of sounds, of the form:
 *            {
 *                warmup = "Hyperdrive_Charge",
 *                abort = "Hyperdrive_Abort",
 *                jump = "Hyperdrive_Jump",
 *            }
 *            The values refer to sound-effects under the data/sounds/ directory.
 *
 *   checks - optional. A function that doesn't take any parameter and returns
 *            true as long as all the conditions for hyperspace are met.
 *
 * Result:
 *
 *   status - a <Constants.ShipJumpStatus> string that tells if the ship can
 *            hyperspace and if not, describes the reason
 *
 * Availability:
 *
 *   February 2014
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_initiate_hyperjump_to(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	SystemPath *dest = LuaObject<SystemPath>::CheckFromLua(2);
	int warmup_time = lua_tointeger(l, 3);
	int duration = lua_tointeger(l, 4);

	HyperdriveSoundsTable sounds;
	if (lua_gettop(l) >= 5 && lua_istable(l, 5)) {
		LuaTable t(l, 5);
		sounds.warmup_sound = t.Get("warmup", "Hyperdrive_Charge");
		sounds.abort_sound = t.Get("abort", "Hyperdrive_Abort");
		sounds.jump_sound = t.Get("jump", "Hyperdrive_Jump");
	} else {
		return luaL_error(l, "Ship:InitiateHyperjumpTo() requires a sound table in argument 5.");
	}

	LuaRef checks;
	if (lua_gettop(l) >= 6)
		checks = LuaRef(l, 6);

	Ship::HyperjumpStatus status = s->InitiateHyperjumpTo(*dest, warmup_time, duration, sounds, checks);

	lua_pushstring(l, EnumStrings::GetString("ShipJumpStatus", status));
	return 1;
}

/*
 * Method: AbortHyperjump
 *
 *   Abort the upcoming hyperjump
 *
 * > ship:AbortHyperjump()
 *
 * Availability:
 *
 *   February 2014
 *
 * Status:
 *
 *   experimental
 */
static int l_ship_abort_hyperjump(lua_State *l)
{
	LuaObject<Ship>::CheckFromLua(1)->AbortHyperjump();
	return 0;
}

/*
 * Method: Create
 *
 *   Create a new ship object by type id (string)
 *   The ship is not added to space.
 *   The object is completely controlled by lua.
 *
 * > ship = ship:Create(ship_id)
 *
 * Parameters:
 *
 *   ship_id - The internal id of the ship type.
 *
 * Result:
 *
 *   ship - new ship object
 */
static int l_ship_create(lua_State *l)
{
	auto name = LuaPull<std::string>(l, 1);
	LuaObject<Ship>::CreateInLua(name);
	return 1;
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

/* Method: GetWheelState
 *
 * Return the current state of the landing gear.
 *
 * Returns:
 *
 *    Return 0.0 if the landing gear is currently up, 1.0 if it is down,
 *    and a value in between if it is moving.
 *
 */
static int l_ship_get_wheel_state(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	lua_pushnumber(l, s->GetWheelState());
	return 1;
}

/* Method: GetFlightState
 *
 * Return the current flight state of a ship as a string.
 *
 * Returns:
 *
 *    One of
 *        - FLYING
 *        - DOCKING
 *        - UNDOCKING
 *        - DOCKED
 *        - LANDED
 *        - JUMPING
 *        - HYPERSPACE
 *
 */
static int l_ship_get_flight_state(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaPush(l, EnumStrings::GetString("ShipFlightState", s->GetFlightState()));
	return 1;
}

/* Method: GetFlightControlState
 *
 * Return the current flight control state.
 *
 * > local state = ship:GetFlightControlState()
 *
 * Returns:
 *
 *    the current flight control state as a string, one of
 *        - CONTROL_MANUAL
 *        - CONTROL_FIXSPEED
 *        - CONTROL_FIXHEADING_FORWARD
 *        - CONTROL_FIXHEADING_BACKWARD
 *        - CONTROL_FIXHEADING_NORMAL
 *        - CONTROL_FIXHEADING_ANTINORMAL
 *        - CONTROL_FIXHEADING_RADIALLY_INWARD
 *        - CONTROL_FIXHEADING_RADIALLY_OUTWARD
 *        - CONTROL_FIXHEADING_KILLROT
 *        - CONTROL_AUTOPILOT
 *
 */
static int l_ship_get_flight_control_state(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaPush(l, EnumStrings::GetString("ShipControllerFlightControlState", s->GetController()->GetFlightControlState()));
	return 1;
}

/* Method: SetFlightControlState
 *
 * Set the ship's flight control state.
 *
 * > ship:SetFlightControlState("CONTROL_MANUAL")
 *
 * Parameters:
 *
 *    state - the new flight control state as a string, one of
 *        - CONTROL_MANUAL
 *        - CONTROL_FIXSPEED
 *        - CONTROL_FIXHEADING_FORWARD
 *        - CONTROL_FIXHEADING_BACKWARD
 *        - CONTROL_FIXHEADING_NORMAL
 *        - CONTROL_FIXHEADING_ANTINORMAL
 *        - CONTROL_FIXHEADING_RADIALLY_INWARD
 *        - CONTROL_FIXHEADING_RADIALLY_OUTWARD
 *        - CONTROL_FIXHEADING_KILLROT
 *        - CONTROL_AUTOPILOT
 *
 */
static int l_ship_set_flight_control_state(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	FlightControlState state = static_cast<FlightControlState>(
		EnumStrings::GetValue("ShipControllerFlightControlState", luaL_checkstring(l, 2)));

	s->GetController()->SetFlightControlState(state);
	return 0;
}

/* Method: GetCruiseSpeed
 *
 * Return the current cruise speed in m/s.
 *
 * local speed = ship:GetCruiseSpeed()
 *
 * Returns:
 *
 *    the current cruise speed in m/s or nil if not in cruise speed mode.
 *
 */
static int l_ship_get_cruise_speed(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	if (s->GetController()->GetFlightControlState() == CONTROL_FIXSPEED)
		LuaPush(l, s->GetController()->GetCruiseSpeed());
	else
		lua_pushnil(l);
	return 1;
}

/* Method: GetFollowTarget
 *
 * Return the current follow target of the ship.
 *
 * Returns:
 *
 *    The <Body> of the current follow target or nil.
 *
 */
static int l_ship_get_follow_target(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	Body *t = s->GetController()->GetFollowTarget();
	/*
	if (s->GetType() == ObjectType::PLAYER && t == nullptr) {
		FrameId fId = s->GetFrame();
		Frame *f = Frame::GetFrame(fId);
		if (f)
			t = f->GetBody();
	}
	*/
	if (t)
		LuaObject<Body>::PushToLua(t);
	else
		lua_pushnil(l);
	return 1;
}

/* Method: ToggleWheelState
 *
 * If the landing gear of the ship is up, start lowering it, and vice versa.
 *
 * > ship:ToggleWheelState()
 *
 */
static int l_ship_toggle_wheel_state(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	s->SetWheelState(is_equal_exact(s->GetWheelState(), 0.0f));
	return 0;
}

/*
 * Method: GetVelocity
 *
 * Get the ships velocity
 *
 * > ship:GetVelocity()
 *
 * Availability:
 *
 *  April 2016
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_get_velocity(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	vector3d v = s->GetVelocity();
	LuaPush<vector3d>(l, v);
	return 1;
}

/* Method: GetStats
 *
 * Return some ship stats.
 *
 * Returns:
 *
 *    Return a table containing:
 *          - usedCapacity
 *          - usedCargo
 *          - freeCapacity
 *          - staticMass
 *          - hullMassLeft
 *          - hyperspaceRange
 *          - hyperspaceRangeMax
 *          - shieldMass
 *          - shieldMassLeft
 *          - fuelTankMassLeft
 *
 */
static int l_ship_get_stats(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaTable t(l, 0, 10);
	const shipstats_t &stats = s->GetStats();
	t.Set("usedCapacity", stats.used_capacity);
	t.Set("usedCargo", stats.used_cargo);
	t.Set("freeCapacity", stats.free_capacity);
	t.Set("staticMass", stats.static_mass);
	t.Set("hullMassLeft", stats.hull_mass_left);
	t.Set("hyperspaceRange", stats.hyperspace_range);
	t.Set("hyperspaceRangeMax", stats.hyperspace_range_max);
	t.Set("shieldMass", stats.shield_mass);
	t.Set("shieldMassLeft", stats.shield_mass_left);
	t.Set("fuelTankMassLeft", stats.fuel_tank_mass_left);
	return 1;
}

/*
 * Method: GetPosition
 *
 * Get the ship's position
 *
 * > ship:GetPosition()
 *
 * Availability:
 *
 *  April 2016
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_get_position(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	vector3d v = s->GetPosition();
	lua_newtable(l);
	pi_lua_settable(l, "x", v.x);
	pi_lua_settable(l, "y", v.y);
	pi_lua_settable(l, "z", v.z);
	return 1;
}

/* Method: GetHullTemperature
 *
 * Return the current hull temperature (0.0 - 1.0).
 *
 * Returns:
 *
 *    the hull temperature (0.0 - 1.0)
 *
 */
static int l_ship_get_hull_temperature(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaPush(l, s->GetHullTemperature());
	return 1;
}

/* Method: GetGunTemperature
 *
 * Get a gun's temperature (0.0 - 1.0).
 *
 * Parameters:
 *
 *    gun_index - the index of the gun (0 for front, 1 for rear)
 *
 * Returns:
 *
 *    the gun's current temperature (0.0 - 1.0)
 *
 */
static int l_ship_get_gun_temperature(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	int gun = luaL_checkinteger(l, 2);
	LuaPush(l, s->GetComponent<FixedGuns>()->GetGunTemperature(gun));
	return 1;
}

/* Method: GetHullPercent
 *
 * Return the current percentage of hull left (0.0 - 1.0).
 *
 * Returns:
 *
 *    the current hull (0.0 - 1.0)
 *
 */
static int l_ship_get_hull_percent(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaPush(l, s->GetPercentHull());
	return 1;
}

/* Method: GetShieldPercent
 *
 * Return the current percentage of shield left (0.0 - 1.0) or nil if no shield.
 *
 * Returns:
 *
 *    the current shield (0.0 - 1.0) or nil
 *
 */
static int l_ship_get_shields_percent(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	double shields = s->GetPercentShields();
	if (s->GetStats().shield_mass <= 0)
		lua_pushnil(l);
	else
		lua_pushnumber(l, shields);
	return 1;
}

/* Method: IsDocked
 *
 * Return true if the ship is docked.
 *
 * Returns:
 *
 *    true if the ship is docked
 *
 */
static int l_ship_is_docked(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	lua_pushboolean(l, s->IsDocked());
	return 1;
}

/* Method: IsLanded
 *
 * Return true if the ship is landed.
 *
 * Returns:
 *
 *    true if the ship is landed
 *
 */
static int l_ship_is_landed(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	lua_pushboolean(l, s->IsLanded());
	return 1;
}

/* Method: GetHyperspaceCountdown
 *
 * Return the current hyperspace countdown in seconds.
 *
 * Returns:
 *
 *    the current hyperspace countdown in seconds.
 *
 */
static int l_ship_get_hyperspace_countdown(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaPush(l, s->GetHyperspaceCountdown());
	return 1;
}

/* Method: GetHyperspaceDestination
 *
 * Return the current hyperspace destination.
 *
 * Returns:
 *
 *    path - the <SystemPath> of the destination
 *    name - a string of the name of the destination
 *
 */
static int l_ship_get_hyperspace_destination(lua_State *l)
{
	Ship *ship = LuaObject<Ship>::CheckFromLua(1);
	const SystemPath &dest = ship->GetHyperspaceDest();
	RefCountedPtr<const Sector> s = Pi::game->GetGalaxy()->GetSector(dest);
	LuaObject<SystemPath>::PushToLua(dest);
	LuaPush(l, s->m_systems[dest.systemIndex].GetName());
	return 2;
}

/* Method: IsHyperspaceActive
 *
 * Return true if a hyperspace countdown is currently running.
 *
 * Returns:
 *
 *    true if a hyperspace countdown is currently running.
 *
 */
static int l_ship_is_hyperspace_active(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	LuaPush<bool>(l, s->IsHyperspaceActive());
	return 1;
}

/* Method: GetThrusterState
 *
 * Return the state of the directional thrusters.
 *
 * Returns:
 *
 *    the state of the directional thrusters
 *
 */
static int l_ship_get_thruster_state(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	vector3d v = s->GetComponent<Propulsion>()->GetLinThrusterState();
	LuaPush<vector3d>(l, v);
	return 1;
}

/* Method: GetThrusterAcceleration
 *
 * Returns maximum acceleration in one of 6 directions
 *
 * Parameters:
 *
 *   direction - number, the <ShipTypeThruster> enum value
 *
 * Returns:
 *
 *   acceleration - number, m/s/s
 *
 */
static int l_ship_get_thruster_acceleration(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	auto direction = Thruster(LuaPull<int>(l, 2));
	double acc = s->GetComponent<Propulsion>()->GetAccel(direction);
	LuaPush(l, acc);
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
 * etc). It is the responsibility of the script to take those additional
 * actions if desired.
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
	Ship *target = LuaObject<Ship>::GetFromLua(2);
	if (target != nullptr) {
		s->AIKill(target);
		lua_pushboolean(l, true);
	} else {
		lua_pushboolean(l, false);
	}
	return 1;
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
 * Returns:
 *   true if the command could be enacted, false otherwise
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
	if (target != nullptr) {
		s->AIKamikaze(target);
		lua_pushboolean(l, true);
	} else {
		lua_pushboolean(l, false);
	}
	return 1;
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
	if (!target->IsType(ObjectType::PLANET) && !target->IsType(ObjectType::STAR))
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
	if (!target->IsType(ObjectType::PLANET) && !target->IsType(ObjectType::STAR))
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
	if (!target->IsType(ObjectType::PLANET) && !target->IsType(ObjectType::STAR))
		luaL_argerror(l, 2, "expected a Planet or a Star");
	s->AIOrbit(target, 3.5);
	return 0;
}

static int l_ship_get_current_ai_command(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	const AICommand *cmd = s->GetAICommand();
	if (cmd != nullptr) {
		LuaPush(l, EnumStrings::GetString("ShipAICmdName", cmd->GetType()));
		return 1;
	} else {
		lua_pushnil(l);
		return 1;
	}
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

/*
 * Method: UpdateEquipStats
 *
 * Update the ship's statistics with regards to equipment changes
 *
 * > ship:UpdateEquipStats()
 *
 * Availability:
 *
 *  June 2014
 *
 * Status:
 *
 *  experimental
 */
static int l_ship_update_equip_stats(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	s->UpdateLuaStats();
	return 0;
}

static int l_ship_attr_equipset(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	s->GetEquipSet().PushCopyToStack();
	return 1;
}

template <>
const char *LuaObject<Ship>::s_type = "Ship";

template <>
void LuaObject<Ship>::RegisterClass()
{
	static const char *l_parent = "ModelBody";

	static const luaL_Reg l_methods[] = {
		{ "IsPlayer", l_ship_is_player },

		{ "GetShipClass", l_ship_get_ship_class },
		{ "GetShipType", l_ship_get_ship_type },
		{ "SetShipType", l_ship_set_type },
		{ "SetHullPercent", l_ship_set_hull_percent },
		{ "SetFuelPercent", l_ship_set_fuel_percent },

		{ "GetSkin", l_ship_get_skin },
		{ "SetSkin", l_ship_set_skin },
		{ "SetPattern", l_ship_set_pattern },
		{ "SetLabel", l_ship_set_label },
		{ "SetShipName", l_ship_set_ship_name },

		{ "GetHyperspaceDestination", l_ship_get_hyperspace_destination },

		{ "SpawnCargo", l_ship_spawn_cargo },

		{ "SpawnMissile", l_ship_spawn_missile },

		{ "UseECM", l_ship_use_ecm },
		{ "IsECMReady", l_ship_is_ecm_ready },
		{ "GetDurationForDistance", l_ship_get_duration_for_distance },

		{ "GetDockedWith", l_ship_get_docked_with },
		{ "SetDockedWith", l_ship_set_docked_with },
		{ "Undock", l_ship_undock },
		{ "BlastOff", l_ship_blast_off },

		{ "Explode", l_ship_explode },

		{ "AIKill", l_ship_ai_kill },
		{ "AIKamikaze", l_ship_ai_kamikaze },
		{ "AIFlyTo", l_ship_ai_fly_to },
		{ "AIDockWith", l_ship_ai_dock_with },
		{ "AIEnterLowOrbit", l_ship_ai_enter_low_orbit },
		{ "AIEnterMediumOrbit", l_ship_ai_enter_medium_orbit },
		{ "AIEnterHighOrbit", l_ship_ai_enter_high_orbit },
		{ "CancelAI", l_ship_cancel_ai },

		{ "InitiateHyperjumpTo", l_ship_initiate_hyperjump_to },
		{ "AbortHyperjump", l_ship_abort_hyperjump },

		{ "Create", l_ship_create },

		{ "GetInvulnerable", l_ship_get_invulnerable },
		{ "SetInvulnerable", l_ship_set_invulnerable },

		{ "UpdateEquipStats", l_ship_update_equip_stats },

		{ "GetVelocity", l_ship_get_velocity },
		{ "GetPosition", l_ship_get_position },
		{ "GetThrusterState", l_ship_get_thruster_state },
		{ "GetThrusterAcceleration", l_ship_get_thruster_acceleration },

		{ "IsDocked", l_ship_is_docked },
		{ "IsLanded", l_ship_is_landed },

		{ "GetWheelState", l_ship_get_wheel_state },
		{ "ToggleWheelState", l_ship_toggle_wheel_state },
		{ "GetFlightState", l_ship_get_flight_state },
		{ "GetCruiseSpeed", l_ship_get_cruise_speed },
		{ "GetFollowTarget", l_ship_get_follow_target },
		{ "GetStats", l_ship_get_stats },

		{ "GetHyperspaceCountdown", l_ship_get_hyperspace_countdown },
		{ "IsHyperspaceActive", l_ship_is_hyperspace_active },

		{ "GetHullTemperature", l_ship_get_hull_temperature },
		{ "GetGunTemperature", l_ship_get_gun_temperature },
		{ "GetHullPercent", l_ship_get_hull_percent },
		{ "GetShieldsPercent", l_ship_get_shields_percent },

		{ "GetFlightControlState", l_ship_get_flight_control_state },
		{ "SetFlightControlState", l_ship_set_flight_control_state },
		{ "GetCurrentAICommand", l_ship_get_current_ai_command },

		{ 0, 0 }
	};

	const luaL_Reg l_attrs[] = {
		{ "equipSet", l_ship_attr_equipset },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
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
 * Attribute: staticMass
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
