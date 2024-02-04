// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EnumStrings.h"
#include "Frame.h"
#include "Game.h"
#include "LuaConstants.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaVector.h"
#include "Pi.h"
#include "Player.h"
#include "SectorView.h"
#include "TerrainBody.h"
#include "WorldView.h"
#include "galaxy/Galaxy.h"
#include "ship/PlayerShipController.h"

/*
 * Class: Player
 *
 * Class representing the player. Inherits from <Ship>
 */

static int l_player_is_player(lua_State *l)
{
	lua_pushboolean(l, true);
	return 1;
}

/*
 * Method: GetNavTarget
 *
 * Get the player's navigation target
 *
 * > target = player:GetNavTarget()
 *
 * Return:
 *
 *   target - nil, or a <Body>
 *
 * Availability:
 *
 *   alpha 15
 *
 * Status:
 *
 *   stable
 */
static int l_get_nav_target(lua_State *l)
{
	Player *p = LuaObject<Player>::CheckFromLua(1);
	LuaObject<Body>::PushToLua(p->GetNavTarget());
	return 1;
}

/*
 * Method: SetNavTarget
 *
 * Set the player's navigation target
 *
 * > player:SetNavTarget(target)
 *
 * Parameters:
 *
 *   target - a <Body> to which to set the navigation target, or nil
 *
 * Availability:
 *
 *   alpha 14
 *
 * Status:
 *
 *   stable
 */
static int l_set_nav_target(lua_State *l)
{
	Player *p = LuaObject<Player>::CheckFromLua(1);
	Body *target = LuaObject<Body>::GetFromLua(2);
	p->SetNavTarget(target);
	return 0;
}

/*
 * Function: SetFollowTarget
 *
 * Set the "reference target" of player's ship
 *
 * Example:
 *
 * > player:SetFollowTarget(body)
 *
 * Parameters:
 *
 *   body - <Body> relative to witch speed, orientation (depending on the mode)
 *   is set relative to
 *
 */
static int l_set_follow_target(lua_State *l)
{
	Player *p = LuaObject<Player>::CheckFromLua(1);
	Body *target = LuaObject<Body>::GetFromLua(2);
	p->SetFollowTarget(target);
	return 0;
}

/*
 * Function: ChangeCruiseSpeed
 *
 * Set the "cruise speed" of player's ship
 *
 * Example:
 *
 * > player:ChangeCruiseSpeed(delta)
 *
 * Parameters:
 *
 *   delta - Float, by how much to change current cruise speed
 *
 */
static int l_change_cruise_speed(lua_State *l)
{
	Player *p = LuaObject<Player>::CheckFromLua(1);
	double delta = LuaPull<double>(l, 2);
	p->ChangeCruiseSpeed(delta);
	return 0;
}

/*
 * Method: GetCombatTarget
 *
 * Get the player's combat target
 *
 * > target = player:GetCombatTarget()
 *
 * Return:
 *
 *   target - nil, or a <Body>
 *
 * Availability:
 *
 *   alpha 15
 *
 * Status:
 *
 *   stable
 */
static int l_get_combat_target(lua_State *l)
{
	Player *p = LuaObject<Player>::CheckFromLua(1);
	LuaObject<Body>::PushToLua(p->GetCombatTarget());
	return 1;
}

/*
 * Method: SetCombatTarget
 *
 * Set the player's combat target
 *
 * > player:SetCombatTarget(target)
 *
 * Parameters:
 *
 *   target - a <Body> to which to set the combat target, or nil
 *
 * Availability:
 *
 *   alpha 14
 *
 * Status:
 *
 *   stable
 */
static int l_set_combat_target(lua_State *l)
{
	Player *p = LuaObject<Player>::CheckFromLua(1);
	Body *target = LuaObject<Body>::GetFromLua(2);
	p->SetCombatTarget(target);
	return 0;
}

/*
 * Method: GetHyperspaceTarget
 *
 * Get the player's hyperspace target
 *
 * > target = player:GetHyperspaceTarget()
 *
 * Return:
 *
 *   target - nil, or a <SystemPath>
 *
 * Availability:
 *
 *   alpha 32
 *
 * Status:
 *
 *   stable
 */
static int l_get_hyperspace_target(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	SystemPath target;
	if (Pi::game->IsNormalSpace())
		target = Pi::game->GetSectorView()->GetHyperspaceTarget();
	else
		target = player->GetHyperspaceDest();
	LuaObject<SystemPath>::PushToLua(target);
	return 1;
}

/*
 * Method: SetHyperspaceTarget
 *
 * Set the player's hyperspace target
 *
 * > player:SetHyperspaceTarget(target)
 *
 * Parameters:
 *
 *   target - a <SystemPath> to which to set the hyperspace target. Must be a system path or the path of a star.
 *
 * Availability:
 *
 *   alpha 32
 *
 * Status:
 *
 *   stable
 */
static int l_set_hyperspace_target(lua_State *l)
{
	LuaObject<Player>::CheckFromLua(1);
	if (Pi::game->IsNormalSpace()) {
		const SystemPath path = *LuaObject<SystemPath>::CheckFromLua(2);
		if (!path.IsSystemPath()) {
			if (!path.IsBodyPath()) {
				return luaL_error(l, "Player:SetHyperspaceTarget() -- second parameter is not a system path or the path of a star");
			}
			RefCountedPtr<StarSystem> sys = Pi::game->GetGalaxy()->GetStarSystem(path);
			// Lua should never be able to get an invalid SystemPath
			// (note: this may change if it becomes possible to remove systems during the game)
			assert(path.bodyIndex < sys->GetNumBodies());
			SystemBody *sbody = sys->GetBodyByPath(path);
			if (sbody->GetSuperType() != SystemBody::SUPERTYPE_STAR)
				return luaL_error(l, "Player:SetHyperspaceTarget() -- second parameter is not a system path or the path of a star");
		}
		Pi::game->GetSectorView()->SetHyperspaceTarget(path);
		return 0;
	} else
		return luaL_error(l, "Player:SetHyperspaceTarget() cannot be used while in hyperspace");
}

static int l_get_mouse_direction(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush<vector3d>(l, player->GetPlayerController()->GetMouseViewDir());
	return 1;
}

/*
 * Method: IsMouseActive
 *
 * Return true if the player is using the mouse to rotate the ship (typically RMB held)
 *
 * > player:IsMouseActive()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_get_is_mouse_active(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush(l, player->GetPlayerController()->IsMouseActive());
	return 1;
}

/*
 * Method: GetMaxDeltaV
 *
 * Get the player's ship's maximum Δv (excluding hydrogen in cargo space)
 *
 * > player:GetMaxDeltaV()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_get_max_delta_v(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	const ShipType *st = player->GetShipType();
	LuaPush(l, st->effectiveExhaustVelocity * log((double(player->GetStats().static_mass + st->fuelTankMass)) / (player->GetStats().static_mass)));
	return 1;
}

/*
 * Method: GetCurrentDeltaV
 *
 * Get the player's ship's current Δv capacity (excluding hydrogen in cargo space)
 *
 * > player:GetCurrentDeltaV()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_get_current_delta_v(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush(l, player->GetVelocityRelTo(player->GetFrame()).Length());
	return 1;
}

/*
 * Method: GetRemainingDeltaV
 *
 * Get the player's ship's remaining Δv capacity (excluding hydrogen in cargo space)
 *
 * > player:GetRemainingDeltaV()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_get_remaining_delta_v(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	const double fuelmass = 1000 * player->GetShipType()->fuelTankMass * player->GetFuel();
	double remaining = player->GetShipType()->effectiveExhaustVelocity * log(player->GetMass() / (player->GetMass() - fuelmass));

	LuaPush(l, remaining);
	return 1;
}

/*
 * Method: GetAcceleration
 *
 * Get the player's ship's current acceleration in a direction.
 *
 * > player:GetAcceleration
 *
 * Parameters:
 *
 *   thruster - a string specifying which thruster's acceleration to return. One of "forward", "reverse" or "up"
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static std::map<std::string, Thruster> thrusters_map = {
	{ "forward", THRUSTER_FORWARD },
	{ "reverse", THRUSTER_REVERSE },
	{ "up", THRUSTER_UP },
};

static int l_get_acceleration(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	std::string thruster = LuaPull<std::string>(l, 2);
	double acceleration = player->GetAccel(thrusters_map.at(thruster));
	LuaPush(l, acceleration);
	return 1;
}

/*
 * Method: GetDistanceToZeroV
 *
 * Get the distance the player's ship needs to decelerate with thruster from speed
 *
 * > player:GetDistanceToZeroV(15000, "forward")
 *
 * Parameters:
 *
 *   speed - speed in m/s
 *   thruster - a string specifying which thruster to user for deceleration. One of "forward", "reverse"
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_get_distance_to_zero_v(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	double speed = LuaPull<double>(l, 2);
	std::string thruster = LuaPull<std::string>(l, 3);
	double acceleration = player->GetAccel(thrusters_map.at(thruster));
	// approximation, ignores mass change and gravity
	LuaPush(l, speed * speed / (2 * acceleration));
	return 1;
}

/*
 * Method: GetManeuverTime
 *
 * Get the time remaining until start of maneuver in seconds
 *
 * > player:GetManeuverTime()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_get_maneuver_time(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush(l, player->GetManeuverTime());
	return 1;
}

/*
 * Method: GetManeuverVelocity
 *
 * Get the current maneuver velocity in m/s
 *
 * > player:GetManeuverVelocity()
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_get_maneuver_velocity(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	vector3d pos = player->GetManeuverVelocity();
	LuaPush(l, pos);
	return 1;
}

/*
 * Method: GetHeadingPitchRoll
 *
 * Get the player's ship's current heading, pitch and roll (all in radians)
 *
 * > heading,pitch,roll = player:GetHeadingPitchRoll("planet")
 *
 * Parameters:
 *
 *   type - "system-wide" or "planet"
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */
static int l_get_heading_pitch_roll(lua_State *l)
{
	//  Player *player = LuaObject<Player>::CheckFromLua(1);
	std::string type = LuaPull<std::string>(l, 2);
	PlaneType pt = PlaneType::PARENT;
	if (!type.compare("system-wide")) {
		pt = PlaneType::PARENT;
	} else if (!type.compare("planet")) {
		pt = PlaneType::ROTATIONAL;
	} else {
		Output("LuaPlayer: l_get_heading_pitch_roll called with unknown type %s\n", type.c_str());
		return 0;
	}

	std::tuple<double, double, double> res = Pi::game->GetWorldView()->CalculateHeadingPitchRoll(pt);
	LuaPush(l, std::get<0>(res));
	LuaPush(l, std::get<1>(res));
	LuaPush(l, std::get<2>(res));
	return 3;
}

/*
 * Function: SetRotationDamping
 *
 * Set rotation dampening on or off of player's ship
 *
 * Example:
 *
 * > player:SetRotationDamping(is_on)
 *
 * Parameters:
 *
 *   is_on - boolean
 *
 */
static int l_set_rotation_damping(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	bool rot = LuaPull<bool>(l, 2);
	player->GetPlayerController()->SetRotationDamping(rot);
	return 0;
}

/*
 * Function: GetRotationDamping
 *
 * Get rotation dampening state of player's ship
 *
 * Example:
 *
 * > state = player:GetRotationDamping()
 *
 * Returns:
 *
 *   state - bool
 *
 */
static int l_get_rotation_damping(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush<bool>(l, player->GetPlayerController()->GetRotationDamping());
	return 1;
}

/*
 * Function: ToggleRotationDamping
 *
 * Toggle rotation dampening on/off
 *
 * Example:
 *
 * > player:ToggleRotationDamping()
 *
 */
static int l_toggle_rotation_damping(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	player->GetPlayerController()->ToggleRotationDamping();
	return 0;
}

static int l_get_alert_state(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	Ship::AlertState state = player->GetAlertState();
	switch (state) {
	case Ship::AlertState::ALERT_NONE:
		lua_pushnil(l);
		break;
	case Ship::AlertState::ALERT_SHIP_NEARBY:
		LuaPush(l, "ship-nearby");
		break;
	case Ship::AlertState::ALERT_SHIP_FIRING:
		LuaPush(l, "ship-firing");
		break;
	case Ship::AlertState::ALERT_MISSILE_DETECTED:
		LuaPush(l, "ship-firing");
		break;
	default:
		Error("Unknown alert state %i", int(state));
	}
	return 1;
}

static int l_get_low_thrust_power(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush(l, player->GetPlayerController()->GetLowThrustPower());
	return 1;
}

static int l_set_low_thrust_power(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	double thrust = luaL_checknumber(l, 2);
	player->GetPlayerController()->SetLowThrustPower(thrust);
	return 0;
}

static int l_player_get_hyperspace_countdown(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush(l, player->GetHyperspaceCountdown());
	return 1;
}

static int l_player_is_hyperspace_active(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush(l, player->IsHyperspaceActive());
	return 1;
}

/*
 * Function: GetCruiseDirection()
 *
 * Returns:
 *
 *   string - 'CRUISE_UP' or 'CRUISE_FWD'
 *
 */
static int l_player_get_cruise_direction(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	auto mode = player->GetPlayerController()->GetCruiseDirection();
	auto str = EnumStrings::GetString("CruiseDirection", mode);
	assert(str && "Wrong get cruise direction");
	LuaPush<const char *>(l, str);
	return 1;
}

/*
 * Function: SetCruiseDirection(direction)
 *
 * Parameters:
 *
 *   direction - a string 'CRUISE_UP' or 'CRUISE_FWD'
 *
 * Returns:
 *
 *   nothing
 *
 */
static int l_player_set_cruise_direction(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	auto dir_name = LuaPull<const char *>(l, 2);
	int value = EnumStrings::GetValue("CruiseDirection", dir_name);
	if (value == -1)
		luaL_error(l, "Player:SetCruiseDirection(): invalid cruise direction '%s' specified\n", dir_name);
	auto dir = static_cast<PlayerShipController::CruiseDirection>(value);
	player->GetPlayerController()->SetCruiseDirection(dir);
	return 0;
}

/*
 * Function: GetFollowMode()
 *
 * Returns:
 *
 *   string - 'FOLLOW_POS' or 'FOLLOW_ORI'
 *
 */
static int l_player_get_follow_mode(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	auto mode = player->GetPlayerController()->GetFollowMode();
	auto str = EnumStrings::GetString("FollowMode", mode);
	assert(str && "Wrong get follow mode");
	LuaPush<const char *>(l, str);
	return 1;
}

/*
 * Function: SetFollowMode(mode)
 *
 * Parameters:
 *
 *   mode - a string 'FOLLOW_POS' or 'FOLLOW_ORI'
 *
 * Returns:
 *
 *   nothing
 *
 */
static int l_player_set_follow_mode(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	auto mode_name = LuaPull<const char *>(l, 2);
	auto value = EnumStrings::GetValue("FollowMode", mode_name);
	if (value == -1)
		luaL_error(l, "Player:SetFollowMode(): invalid follow mode '%s' specified\n", mode_name);
	auto mode = static_cast<PlayerShipController::FollowMode>(value);
	player->GetPlayerController()->SetFollowMode(mode);
	return 0;
}

/*
 * Function: GetSpeedLimit(speed_limit)
 *
 * Returns:
 *
 *   number - positive, in manual mode, limits the real speed of the
 *            ship in the current frame, in cruise mode limits the amount of cruise
 *            speed. If the limiter is off, nil is returned.
 *
 */
static int l_player_get_speed_limit(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	if (player->GetPlayerController()->IsSpeedLimiterActive()) {
		LuaPush(l, player->GetPlayerController()->GetSpeedLimit());
	} else {
		lua_pushnil(l);
	}
	return 1;
}

/*
 * Function: SetSpeedLimit(speed_limit)
 *
 * Parameters:
 *
 *   speed_limit - a number, must be positive, 0 means no limit.
 *                 in manual mode, limits the real speed of the ship in the
 *                 current frame, in cruise mode limits the amount of cruise
 *                 speed
 *
 * Returns:
 *
 *   nothing
 *
 */
static int l_player_set_speed_limit(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	int s = LuaPull<int>(l, 2);
	player->GetPlayerController()->SetSpeedLimit(s);
	return 0;
}

/*
 * Function: SetSpeedLimiterActive(active)
 *
 * Parameters:
 *
 *   active - a boolean
 *
 * Returns:
 *
 *   nothing
 *
 */
static int l_player_set_speed_limiter_active(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	bool active = LuaPull<bool>(l, 2);
	player->GetPlayerController()->SetSpeedLimiterActive(active);
	return 0;
}

/*
 * Function: IsShipDrifting()
 *
 * Returns:
 *
 *   bool - if the current setting of the cruise speed in the current following
 *          mode significantly differs from the real speed of the ship, but not
 *          too much. Criteria is hardcoded in the PlayerShipController.
 *
 */
static int l_player_is_ship_drifting(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush(l, player->GetPlayerController()->IsShipDrifting());
	return 1;
}

template <>
const char *LuaObject<Player>::s_type = "Player";

template <>
void LuaObject<Player>::RegisterClass()
{
	static const char *l_parent = "Ship";

	static const luaL_Reg l_methods[] = {
		{ "IsPlayer", l_player_is_player },

		{ "GetNavTarget", l_get_nav_target },
		{ "SetNavTarget", l_set_nav_target },
		{ "SetFollowTarget", l_set_follow_target },
		{ "ChangeCruiseSpeed", l_change_cruise_speed },
		{ "GetCombatTarget", l_get_combat_target },
		{ "SetCombatTarget", l_set_combat_target },
		{ "GetHyperspaceTarget", l_get_hyperspace_target },
		{ "SetHyperspaceTarget", l_set_hyperspace_target },
		{ "GetDistanceToZeroV", l_get_distance_to_zero_v },
		{ "GetHeadingPitchRoll", l_get_heading_pitch_roll },
		{ "GetMaxDeltaV", l_get_max_delta_v },
		{ "GetCurrentDeltaV", l_get_current_delta_v },
		{ "GetRemainingDeltaV", l_get_remaining_delta_v },
		{ "GetManeuverVelocity", l_get_maneuver_velocity },
		{ "GetManeuverTime", l_get_maneuver_time },
		{ "GetAcceleration", l_get_acceleration },
		{ "IsMouseActive", l_get_is_mouse_active },
		{ "GetMouseDirection", l_get_mouse_direction },
		{ "GetRotationDamping", l_get_rotation_damping },
		{ "SetRotationDamping", l_set_rotation_damping },
		{ "ToggleRotationDamping", l_toggle_rotation_damping },
		{ "GetAlertState", l_get_alert_state },
		{ "GetLowThrustPower", l_get_low_thrust_power },
		{ "SetLowThrustPower", l_set_low_thrust_power },
		{ "IsHyperspaceActive", l_player_is_hyperspace_active },
		{ "GetHyperspaceCountdown", l_player_get_hyperspace_countdown },
		{ "GetCruiseDirection", l_player_get_cruise_direction },
		{ "SetCruiseDirection", l_player_set_cruise_direction },
		{ "GetFollowMode", l_player_get_follow_mode },
		{ "SetFollowMode", l_player_set_follow_mode },
		{ "GetSpeedLimit", l_player_get_speed_limit },
		{ "SetSpeedLimit", l_player_set_speed_limit },
		{ "SetSpeedLimiterActive", l_player_set_speed_limiter_active },
		{ "IsShipDrifting", l_player_is_ship_drifting },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Player>::DynamicCastPromotionTest);
}
