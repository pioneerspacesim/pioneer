// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
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
#include "SystemView.h" // for the transfer planner
#include "LuaPiGui.h" // for luaL_checkbool

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

static int l_get_current_delta_v(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	lua_pushnumber(l, player->GetVelocityRelTo(player->GetFrame()).Length());
	return 1;
}

static int l_get_remaining_delta_v(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	//	double remaining = player->GetSpeedReachedWithFuel();
	const double fuelmass = 1000*player->GetShipType()->fuelTankMass * player->GetFuel();
	double remaining = player->GetShipType()->effectiveExhaustVelocity * log(player->GetMass()/(player->GetMass()-fuelmass));

	lua_pushnumber(l, remaining);
	return 1;
}

static int l_get_max_delta_v(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	const ShipType *st = player->GetShipType();
	// Output("hull Mass: %f, fuel tank mass: %f, exhaust velocity: %f\nmass: %f, static_mass: %f, used_capacity: %f, fuel: %f\n",
	// 			 (float)st->hullMass, (float)st->fuelTankMass, (float)st->effectiveExhaustVelocity,
	// 			 (float)player->GetMass(), (float)player->GetStats().static_mass, (float)player->GetStats().used_capacity, (float)player->GetFuel());
	lua_pushnumber(l, st->effectiveExhaustVelocity * log((double(player->GetStats().static_mass + st->fuelTankMass)) / (player->GetStats().static_mass)));
	return 1;
}

static int l_get_frame(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	LuaObject<Body>::PushToLua(player->GetFrame()->GetBody());
	return 1;
}

static int l_set_rotation_damping(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	bool rot = luaL_checkbool(l, 2);
	player->GetPlayerController()->SetRotationDamping(rot);
	return 0;
}

static int l_get_rotation_damping(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	lua_pushboolean(l, player->GetPlayerController()->GetRotationDamping());
	return 1;
}

static int l_toggle_rotation_damping(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	Output("Toggling\n");
	player->GetPlayerController()->ToggleRotationDamping();
	return 0;
}

static int l_get_oriented_velocity(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	auto vel = player->GetVelocity() * player->GetOrient();
	LuaTable v(l);
	v.Set("x", vel.x);
	v.Set("y", vel.y);
	v.Set("z", vel.z);
	return 1;
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
			lua_pushnumber(l, altitude);
			lua_pushnumber(l, vspeed);
			const float lat = RAD2DEG(asin(surface_pos.y));
			const float lon = RAD2DEG(atan2(surface_pos.x, surface_pos.z));
			std::string lat_str = DecimalToDegMinSec(lat);
			std::string lon_str = DecimalToDegMinSec(lon);
			lua_pushstring(l, lat_str.c_str());
			lua_pushstring(l, lon_str.c_str());
			return 4;
			//				}
	}
}
return 0;
}

static int l_get_orbit(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	auto orbit = player->ComputeOrbit();
	double eccentricity = orbit.GetEccentricity();
	lua_pushnumber(l, eccentricity);
	lua_pushnumber(l, orbit.GetSemiMajorAxis());
	lua_pushnumber(l, orbit.GetInclination());
	if(eccentricity <= 1.0)
		lua_pushnumber(l, orbit.Period());
	else
		lua_pushnil(l);
	auto aa = orbit.Apogeum();
	lua_pushnumber(l, orbit.OrbitalTimeAtPos(aa, player->GetFrame()->GetNonRotFrame()->GetSystemBody()->GetMass()));
	LuaTable apoapsis(l);
	apoapsis.Set("x", aa.x);
	apoapsis.Set("y", aa.y);
	apoapsis.Set("z", aa.z);
	auto pa = orbit.Perigeum();
	lua_pushnumber(l, orbit.OrbitalTimeAtPos(pa, player->GetFrame()->GetNonRotFrame()->GetSystemBody()->GetMass()));
	LuaTable periapsis(l);
	periapsis.Set("x", pa.x);
	periapsis.Set("y", pa.y);
	periapsis.Set("z", pa.z);

	return 8;
}

static int l_get_low_thrust_power(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	lua_pushnumber(l, player->GetPlayerController()->GetLowThrustPower());
	return 1;
}

static int l_set_low_thrust_power(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	double thrust = luaL_checknumber(l, 2);
	player->GetPlayerController()->SetLowThrustPower(thrust);
	return 0;
}

static int l_get_heading_pitch(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	std::string type = luaL_checkstring(l, 2);
	PlaneType pt;
	if(!type.compare("system-wide")) {
		pt = PlaneType::PARENT;
	} else if(!type.compare("planet")) {
		pt = PlaneType::ROTATIONAL;
	} // TODO: else error
	
	std::pair<double,double> res = Pi::game->GetWorldView()->CalculateHeadingPitch(pt);
	lua_pushnumber(l, res.first);
	lua_pushnumber(l, res.second);
	return 2;
}

static int l_get_maneuver_speed(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);

	// see WorldView.cpp:1688
	if(Pi::planner->GetOffsetVel().ExactlyEqual(vector3d(0,0,0))) {
		return 0;
	} else {
		Orbit playerOrbit = player->ComputeOrbit();
		const SystemBody* systemBody = player->GetFrame()->GetSystemBody();
		if(!is_zero_exact(playerOrbit.GetSemiMajorAxis())) {
			double mass = systemBody->GetMass();
			// XXX The best solution would be to store the mass(es) on Orbit
			const vector3d camSpacePlanSpeed = (Pi::planner->GetVel() - playerOrbit.OrbitalVelocityAtTime(mass, playerOrbit.OrbitalTimeAtPos(Pi::planner->GetPosition(), mass))); // * cam_rot
			double relativeSpeed = camSpacePlanSpeed.Length();
			lua_pushnumber(l, relativeSpeed);
			return 1;
		} else {
			return 0;
		}
	}
}

static int l_get_distance_to_zero_v(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	std::string target = luaL_checkstring(l, 2); // "nav", "frame"
	std::string direction = luaL_checkstring(l, 3); // "prograde", "retrograde"
	double v, a;
	if(!target.compare("nav")) {
		if(!direction.compare("prograde")) {
			v = player->GetVelocityRelTo(player->GetNavTarget()).Length();
			a = player->GetAccelRev();
		} else if(!direction.compare("retrograde")) {
			v = player->GetVelocityRelTo(player->GetNavTarget()).Length();
			a = player->GetAccelFwd();
		} else {
			return 0;
		}
	} else if(!target.compare("frame")) {
		if(!direction.compare("prograde")) {
			v = player->GetVelocityRelTo(player->GetFrame()).Length();
			a = player->GetAccelRev();
		} else if(!direction.compare("retrograde")) {
			v = player->GetVelocityRelTo(player->GetFrame()).Length();
			a = player->GetAccelFwd();
		} else {
			return 0;
		}
	} else
		return 0;
	lua_pushnumber(l, v*v/(2*a));
	return 1;
}

template <> const char *LuaObject<Player>::s_type = "Player";

template <> void LuaObject<Player>::RegisterClass()
{
	static const char *l_parent = "Ship";

	static const luaL_Reg l_methods[] = {
		{ "IsPlayer", l_player_is_player },

		{ "GetNavTarget",    l_get_nav_target    },
		{ "SetNavTarget",    l_set_nav_target    },
		{ "GetCombatTarget", l_get_combat_target },
		{ "SetCombatTarget", l_set_combat_target },
		{ "GetHyperspaceTarget", l_get_hyperspace_target },
		{ "SetHyperspaceTarget", l_set_hyperspace_target },
		{ "GetMaxDeltaV",        l_get_max_delta_v },
		{ "GetCurrentDeltaV",    l_get_current_delta_v },
		{ "GetRemainingDeltaV",  l_get_remaining_delta_v },
		{ "GetDistanceToZeroV",  l_get_distance_to_zero_v },
		{ "GetFrame",            l_get_frame },
		{ "GetOrientedVelocity", l_get_oriented_velocity },
		{ "GetOrbit",            l_get_orbit },
		{ "GetGPS",              l_get_gps },
		{ "GetLowThrustPower",   l_get_low_thrust_power },
		{ "SetLowThrustPower",   l_set_low_thrust_power },
		{ "GetManeuverSpeed",    l_get_maneuver_speed },
		{ "GetRotationDamping",  l_get_rotation_damping },
		{ "SetRotationDamping",  l_set_rotation_damping },
		{ "ToggleRotationDamping",  l_toggle_rotation_damping },
		{ "GetHeadingPitch",     l_get_heading_pitch },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Player>::DynamicCastPromotionTest);
}
