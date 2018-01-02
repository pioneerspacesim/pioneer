// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "Player.h"
#include "Pi.h"
#include "Game.h"
#include "SectorView.h"
#include "EnumStrings.h"
#include "galaxy/Galaxy.h"
#include "LuaPiGui.h"
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

static int l_set_set_speed_target(lua_State *l)
{
	Player *p = LuaObject<Player>::CheckFromLua(1);
	Body *target = LuaObject<Body>::GetFromLua(2);
	p->SetSetSpeedTarget(target);
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
	//		Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush(l, Pi::game->GetWorldView()->GetMouseDirection());
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
	const double fuelmass = 1000*player->GetShipType()->fuelTankMass * player->GetFuel();
	double remaining = player->GetShipType()->effectiveExhaustVelocity * log(player->GetMass()/(player->GetMass()-fuelmass));

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
 *   thruster - a string specifying which thruster's acceleration to return. One of "forward", "reverse"
 *
 * Availability:
 *
 *   2017-04
 *
 * Status:
 *
 *   stable
 */

static std::map<std::string, Thruster> thrusters_map = { { "forward", THRUSTER_FORWARD },
																												 { "reverse", THRUSTER_REVERSE } ,
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
	// approximation, ignores mass change
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
  if(!type.compare("system-wide")) {
		pt = PlaneType::PARENT;
  } else if(!type.compare("planet")) {
		pt = PlaneType::ROTATIONAL;
  } else {
		Output("LuaPlayer: l_get_heading_pitch_roll called with unknown type %s\n", type.c_str());
		return 0;
	}

  std::tuple<double,double,double> res = Pi::game->GetWorldView()->CalculateHeadingPitchRoll(pt);
  LuaPush(l, std::get<0>(res));
  LuaPush(l, std::get<1>(res));
  LuaPush(l, std::get<2>(res));
  return 3;
}

static int l_set_rotation_damping(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	bool rot = LuaPull<bool>(l, 2);
	player->GetPlayerController()->SetRotationDamping(rot);
	return 0;
}

static int l_get_rotation_damping(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaPush<bool>(l, player->GetPlayerController()->GetRotationDamping());
	return 1;
}

static int l_toggle_rotation_damping(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	player->GetPlayerController()->ToggleRotationDamping();
	return 0;
}

static int l_get_gps(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	vector3d pos = Pi::player->GetPosition();
	double center_dist = pos.Length();
	auto frame = player->GetFrame();
	if(frame) {
		Body *astro = frame->GetBody();
		if(astro && astro->IsType(Object::TERRAINBODY)) {
			TerrainBody* terrain = static_cast<TerrainBody*>(astro);
			if (!frame->IsRotFrame())
				frame = frame->GetRotFrame();
			vector3d surface_pos = pos.Normalized();
			double radius = 0.0;
			if (center_dist <= 3.0 * terrain->GetMaxFeatureRadius()) {
				radius = terrain->GetTerrainHeight(surface_pos);
			}
			double altitude = center_dist - radius;
			vector3d velocity = player->GetVelocity();
			double vspeed = velocity.Dot(surface_pos);
			if (fabs(vspeed) < 0.05) vspeed = 0.0; // Avoid alternating between positive/negative zero

			//			RefreshHeadingPitch();

			if (altitude < 0) altitude = 0;
			LuaPush(l, altitude);
			LuaPush(l, vspeed);
			const float lat = RAD2DEG(asin(surface_pos.y));
			const float lon = RAD2DEG(atan2(surface_pos.x, surface_pos.z));
			LuaPush(l, lat);
			LuaPush(l, lon);
			return 4;
			//				}
		}
	}
	return 0;
}

static int l_get_alert_state(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	Ship::AlertState state = player->GetAlertState();
	switch(state) {
	case Ship::AlertState::ALERT_NONE:
		lua_pushnil(l);
		break;
	case Ship::AlertState::ALERT_SHIP_NEARBY:
		LuaPush(l, "ship-nearby");
		break;
	case Ship::AlertState::ALERT_SHIP_FIRING:
		LuaPush(l, "ship-firing");
		break;
	default:
		Error("Unknown alert state %i", state);
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

template <> const char *LuaObject<Player>::s_type = "Player";

template <> void LuaObject<Player>::RegisterClass()
{
	static const char *l_parent = "Ship";

	static const luaL_Reg l_methods[] = {
		{ "IsPlayer", l_player_is_player },

		{ "GetNavTarget",        l_get_nav_target    },
		{ "SetNavTarget",        l_set_nav_target    },
		{ "SetSetSpeedTarget",   l_set_set_speed_target },
		{ "GetCombatTarget",     l_get_combat_target },
		{ "SetCombatTarget",     l_set_combat_target },
		{ "GetHyperspaceTarget", l_get_hyperspace_target },
		{ "SetHyperspaceTarget", l_set_hyperspace_target },
		{ "GetDistanceToZeroV",  l_get_distance_to_zero_v },
		{ "GetHeadingPitchRoll", l_get_heading_pitch_roll },
		{ "GetMaxDeltaV",        l_get_max_delta_v },
		{ "GetCurrentDeltaV",    l_get_current_delta_v },
		{ "GetRemainingDeltaV",  l_get_remaining_delta_v },
		{ "GetManeuverVelocity", l_get_maneuver_velocity },
		{ "GetManeuverTime",     l_get_maneuver_time },
		{ "GetAcceleration",     l_get_acceleration },
		{ "IsMouseActive",       l_get_is_mouse_active },
		{ "GetMouseDirection",   l_get_mouse_direction },
		{ "GetRotationDamping",  l_get_rotation_damping },
		{ "SetRotationDamping",  l_set_rotation_damping },
		{ "GetGPS",              l_get_gps },
		{ "ToggleRotationDamping",  l_toggle_rotation_damping },
		{ "GetAlertState",       l_get_alert_state },
		{ "GetLowThrustPower",   l_get_low_thrust_power },
		{ "SetLowThrustPower",   l_set_low_thrust_power },
		{ "IsHyperspaceActive",      l_player_is_hyperspace_active },
		{ "GetHyperspaceCountdown",  l_player_get_hyperspace_countdown },
			
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Player>::DynamicCastPromotionTest);
}
