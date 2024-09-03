// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaSpace.h"
#include "Body.h"
#include "CargoBody.h"
#include "EnumStrings.h"
#include "Frame.h"
#include "Game.h"
#include "HyperspaceCloud.h"
#include "LuaBody.h"
#include "LuaManager.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaVector.h"
#include "MathUtil.h"
#include "Pi.h"
#include "Planet.h"
#include "Player.h"
#include "Ship.h"
#include "Space.h"
#include "SpaceStation.h"
#include "profiler/Profiler.h"
#include "ship/PrecalcPath.h"

/*
 * Interface: Space
 *
 * Various functions to create and find objects in the current physics space.
 */

static void _unpack_hyperspace_args(lua_State *l, int index, SystemPath *&path, double &due)
{
	if (lua_isnone(l, index)) return;

	luaL_checktype(l, index, LUA_TTABLE);

	LUA_DEBUG_START(l);

	lua_pushinteger(l, 1);
	lua_gettable(l, index);
	if (!(path = LuaObject<SystemPath>::GetFromLua(-1)))
		luaL_error(l, "bad value for hyperspace path at position 1 (SystemPath expected, got %s)", luaL_typename(l, -1));
	lua_pop(l, 1);

	lua_pushinteger(l, 2);
	lua_gettable(l, index);
	if (!(lua_isnumber(l, -1)))
		luaL_error(l, "bad value for hyperspace exit time at position 2 (%s expected, got %s)", lua_typename(l, LUA_TNUMBER), luaL_typename(l, -1));
	due = lua_tonumber(l, -1);
	if (due < 0)
		luaL_error(l, "bad value for hyperspace exit time at position 2 (must be >= 0)");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}

// overload for the case when we explicitly want to specify the destination star
static void _unpack_hyperspace_args(lua_State *l, int index, SystemPath *&source, SystemPath *&dest, double &due)
{
	if (lua_isnone(l, index)) return;

	luaL_checktype(l, index, LUA_TTABLE);

	LUA_DEBUG_START(l);

	lua_pushinteger(l, 1);
	lua_gettable(l, index);
	if (!(source = LuaObject<SystemPath>::GetFromLua(-1)))
		luaL_error(l, "bad value for hyperspace source path at position 1 (SystemPath expected, got %s)", luaL_typename(l, -1));
	lua_pop(l, 1);

	lua_pushinteger(l, 2);
	lua_gettable(l, index);
	if (!(dest = LuaObject<SystemPath>::GetFromLua(-1)))
		luaL_error(l, "bad value for hyperspace dest path at position 2 (SystemPath expected, got %s)", luaL_typename(l, -1));
	lua_pop(l, 1);

	lua_pushinteger(l, 3);
	lua_gettable(l, index);
	if (!(lua_isnumber(l, -1)))
		luaL_error(l, "bad value for hyperspace exit time at position 3 (%s expected, got %s)", lua_typename(l, LUA_TNUMBER), luaL_typename(l, -1));
	due = lua_tonumber(l, -1);
	if (due < 0)
		luaL_error(l, "bad value for hyperspace exit time at position 3 (must be >= 0)");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}

static Body *_maybe_wrap_ship_with_cloud(Ship *ship, SystemPath *path, double due)
{
	if (!path) return ship;
	if (due <= 0) return ship;

	HyperspaceCloud *cloud = new HyperspaceCloud(ship, due, true);
	ship->SetHyperspaceDest(path);
	ship->SetFlightState(Ship::HYPERSPACE);

	return cloud;
}

// sb - central systembody, pos - absolute coordinates of given object
static vector3d _orbital_velocity_random_direction(const SystemBody *sb, const vector3d &pos)
{
	// If we got a zero mass of central body - there is no orbit
	if (sb->GetMass() < 0.01)
		return vector3d(0.0);
	// calculating basis from radius - vector
	vector3d k = pos.Normalized();
	vector3d i;
	if (std::fabs(k.z) > 0.999999)	 // very vertical = z
		i = vector3d(1.0, 0.0, 0.0); // second ort = x
	else
		i = k.Cross(vector3d(0.0, 0.0, 1.0)).Normalized();
	vector3d j = k.Cross(i);
	// generating random 2d direction and putting it into basis
	vector3d randomOrthoDirection = MathUtil::RandomPointOnCircle(1.0) * matrix3x3d::FromVectors(i, j, k).Transpose();
	// calculate the value of the orbital velocity
	double orbitalVelocity = sqrt(G * sb->GetMass() / pos.Length());
	return randomOrthoDirection * orbitalVelocity;
}

/*
 * Function: SpawnShip
 *
 * Create a ship and place it somewhere in space.
 *
 * > ship = Space.SpawnShip(type, min, max, hyperspace)
 *
 * Parameters:
 *
 *   type - the name of the ship
 *
 *   min - minimum distance from the system centre (usually the primary star)
 *         to place the ship, in AU
 *
 *   max - maximum distance to place the ship
 *
 *   hyperspace - optional table containing hyperspace entry information. If
 *                this is provided the ship will not spawn directly. Instead,
 *                a hyperspace cloud will be created that the ship will exit
 *                from. The table contains two elements, a <SystemPath> for
 *                the system the ship is travelling from, and the due
 *                date/time that the ship should emerge from the cloud.
 *                In this case min and max arguments are ignored.
 *
 * Return:
 *
 *   ship - a <Ship> object for the new ship
 *
 * Examples:
 *
 * > -- spawn a ship 5-6AU from the system centre
 * > local ship = Space.SpawnShip("eagle_lrf", 5, 6)
 *
 * > -- spawn a ship in the ~11AU hyperspace area and make it appear that it
 * > -- came from Sol and will arrive in ten minutes
 * > local ship = Space.SpawnShip(
 * >     "flowerfairy", 9, 11,
 * >     { SystemPath:New(0,0,0), Game.time + 600 }
 * > )
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_space_spawn_ship(lua_State *l)
{
	if (!Pi::game)
		luaL_error(l, "Game is not started");

	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (!ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);

	float min_dist = luaL_checknumber(l, 2);
	float max_dist = luaL_checknumber(l, 3);

	SystemPath *source = 0;
	SystemPath *dest = 0;
	double due = -1;
	_unpack_hyperspace_args(l, 4, source, dest, due);

	Ship *ship = new Ship(type);
	assert(ship);

	Body *thing = _maybe_wrap_ship_with_cloud(ship, source, due);

	// XXX protect against spawning inside the body
	thing->SetFrame(Pi::game->GetSpace()->GetRootFrame());
	if (!source)
		thing->SetPosition(MathUtil::RandomPointOnSphere(min_dist, max_dist) * AU);
	else
		// XXX broken. this is ignoring min_dist & max_dist. otoh, what's the
		// correct behaviour given there's now a fixed hyperspace exit point?
		thing->SetPosition(Pi::game->GetSpace()->GetHyperspaceExitPoint(*source, *dest));
	thing->SetVelocity(vector3d(0, 0, 0));
	Pi::game->GetSpace()->AddBody(thing);

	LuaObject<Ship>::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

// functions from ShipAiCmd.cpp
extern int CheckCollision(DynamicBody *dBody, const vector3d &pathdir, double pathdist, double targAlt, double endvel, double r);
extern double MaxEffectRad(const Body *body, Propulsion *prop);

/*
 * Function: PutShipOnRoute
 *
 * The ship rearranged from its current position to a given body in space, for a
 * given part of the path, as if it were flying in a straight line, consuming
 * fuel and changing speed. the ship's current speed is ignored and is
 * considered to be equal to target's.
 * This function changes the coordinates, the mass of fuel in the tank, and the
 * speed of the given ship.
 *
 * > Space.PutShipOnRoute(ship, targetbody, t_ratio)
 *
 * Parameters:
 *
 *   ship - a <Ship> object to be moved
 *
 *   targetbody - the <Body> - route goal
 *
 *   t_ratio - route completion rate, by time: 0.0 (begin) ... 1.0 (end)
 *
 * Return:
 *
 *   nothing
 *
 * Examples:
 *
 * > -- move a ship to the middle of the route (by time)
 * > Space.PutShipOnRoute(ship, some_staport, 0.5)
 *
 * Availability:
 *
 *   2020
 *
 * Status:
 *
 *   experimental
 */
static int l_space_put_ship_on_route(lua_State *l)
{
	LUA_DEBUG_START(l);
	Ship *ship = LuaObject<Ship>::CheckFromLua(1);
	const Body *targetbody = LuaObject<Body>::CheckFromLua(2);
	const double t_ratio = LuaPull<double>(l, 3);
	const ShipType *st = ship->GetShipType();
	const shipstats_t ss = ship->GetStats();
	const vector3d route = targetbody->GetPositionRelTo(ship->GetFrame()) - ship->GetPosition();
	PrecalcPath pp(
		route.Length(), // distance
		0.0,			// velocity at start
		st->effectiveExhaustVelocity,
		st->linThrust[THRUSTER_FORWARD],
		st->linAccelerationCap[THRUSTER_FORWARD],
		1000 * (ss.static_mass + ss.fuel_tank_mass_left), // 100% mass of the ship
		1000 * ss.fuel_tank_mass_left * 0.8,			  // multipied to 0.8 have fuel reserve
		0.85);											  // braking margin
	// determine the place of the ship on the route
	pp.setTRatio(t_ratio);
	ship->SetPosition(ship->GetPosition() + route.Normalized() * pp.getDist());
	ship->SetVelocity(route.Normalized() * pp.getVel() + targetbody->GetVelocityRelTo(ship->GetFrame()));
	ship->SetFuel((0.001 * pp.getMass() - ss.static_mass) / st->fuelTankMass);

	ship->UpdateFrame();
	// check for collision at spawn position
	const vector3d shippos = ship->GetPosition();
	const vector3d targpos = targetbody->GetPositionRelTo(ship->GetFrame());
	double targAlt = targpos.Length();
	const vector3d relpos = targpos - shippos;
	const vector3d reldir = relpos.NormalizedSafe();
	const double targdist = relpos.Length();
	Body *body = Frame::GetFrame(ship->GetFrame())->GetBody();
	const double erad = MaxEffectRad(body, ship->GetPropulsion());
	const int coll = CheckCollision(ship, reldir, targdist, targAlt, 0, erad);
	if (coll) {
		// need to correct positon, to avoid collision
		if (targAlt > erad) {
			// target is above the effective radius of obstructor - rotate the ship's position
			// around the target position, so that the obstructor's "effective radius" does not cross the path
			// direction obstructor -> target
			const vector3d z = targpos / targAlt;
			// the axis around which the position of the ship will rotate
			const vector3d y = z.Cross(shippos).NormalizedSafe();
			// just the third axis of this basis
			const vector3d x = y.Cross(z);

			// this is the basis in which the position of the ship will rotate
			const matrix3x3d corrCS = matrix3x3d::FromVectors(x, y, z).Transpose();
			const double len = targAlt;
			// two possible positions of the ship, when flying around the obstructor to the right or left
			// rotate (in the given basis) the direction from the target to the obstructor, so that it passes tangentially to the obstructor
			const vector3d safe1 = corrCS.Transpose() * (matrix3x3d::RotateY(+asin(erad / len)) * corrCS * -targpos).Normalized() * targdist;
			const vector3d safe2 = corrCS.Transpose() * (matrix3x3d::RotateY(-asin(erad / len)) * corrCS * -targpos).Normalized() * targdist;
			// choose the one that is closer to the current position of the ship
			if ((safe1 + relpos).Length() < (safe2 + relpos).Length())
				ship->SetPosition(safe1 + targpos);
			else
				ship->SetPosition(safe2 + targpos);
		} else {
			// target below the effective radius of obstructor. Position the ship direct above the target
			ship->SetPosition(targpos + targpos / targAlt * targdist);
		}
		// update velocity direction
		ship->SetVelocity((targpos - ship->GetPosition()).Normalized() * pp.getVel() + targetbody->GetVelocityRelTo(ship->GetFrame()));
	}
	LUA_DEBUG_END(l, 1);
	return 0;
}

/*
 * Method: PutShipIntoOrbit
 *
 * Puts ship into orbit of target body with SystemBody.
 *
 * > Space.PutShipIntoOrbit(ship, target)
 *
 * Parameters:
 *
 *   ship - a <Ship> object to be moved
 *
 *   target - the <Star> or <Planet> to orbit
 *
 * Availability:
 *
 *  October 2023
 *
 * Status:
 *
 *  experimental
 */
static int l_put_ship_into_orbit(lua_State *l)
{
	Ship *s = LuaObject<Ship>::CheckFromLua(1);
	Body *b = LuaObject<Body>::CheckFromLua(2);
	const SystemBody *sbody = b->GetSystemBody();
	if (!sbody) {
		return luaL_error(l, "the target body doesn't have a system body");
	}
	if (!sbody->GetMass()) {
		return luaL_error(l, "the target body has zero mass");
	}
	if (!b->GetPhysRadius()) {
		return luaL_error(l, "the target body has a zero physical radius");
	}
	Ship::FlightState currentState = s->GetFlightState();
	if (currentState != Ship::FlightState::FLYING) {
		return luaL_error(l, "the ship is not in the \"FLYING\" state. Current state: \"%s\"",
			EnumStrings::GetString("ShipFlightState", currentState));
	}
	// calculations are borrowed from Space::GetHyperspaceExitParams
	// calculate distance to primary body relative to body's mass and radius
	const double max_orbit_vel = 100e3;
	double dist = G * sbody->GetMass() / (max_orbit_vel * max_orbit_vel);
	// ensure an absolute minimum and an absolute maximum distance
	// the minimum distance from the center of the body should not be less than the radius of the body
	// use physical radius, because radius of sbody can be a lot less than physical radius
	double radius = b->GetPhysRadius();
	dist = Clamp(dist, radius * 1.1, std::max(radius * 1.1, 100 * AU));
	vector3d pos{ MathUtil::RandomPointOnSphere(dist) };
	s->SetFrame(b->GetFrame());
	s->SetPosition(pos);
	s->SetVelocity(_orbital_velocity_random_direction(sbody, s->GetPosition()));
	return 0;
}

/*
 * Function: SpawnShipNear
 *
 * Create a ship and place it in space near the given <Body>.
 *
 * > ship = Space.SpawnShipNear(type, body, min, max, hyperspace)
 *
 * Parameters:
 *
 *   type - the name of the ship
 *
 *   body - the <Body> near which the ship should be spawned
 *
 *   min - minimum distance from the body to place the ship, in Km
 *
 *   max - maximum distance to place the ship
 *
 *   hyperspace - optional table containing hyperspace entry information. If
 *                this is provided the ship will not spawn directly. Instead,
 *                a hyperspace cloud will be created that the ship will exit
 *                from. The table contains two elements, a <SystemPath> for
 *                the system the ship is travelling from, and the due
 *                date/time that the ship should emerge from the cloud.
 *
 *   velocity - vector containing the velocity to give to the ship
 *
 *
 * Return:
 *
 *   ship - a <Ship> object for the new ship
 *
 * Examples:
 *
 * > -- spawn a ship 10km from the player
 * > local ship = Space.SpawnShipNear("viper_police_craft", Game.player, 10, 10)
 *
 * > -- spawn a ship 10km from the player with the players velocity
 * > local ship = Space.SpawnShipNear("viper_police_craft", Game.player, 10, 10, nil, Game.player:velocity)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_space_spawn_ship_near(lua_State *l)
{
	if (!Pi::game)
		luaL_error(l, "Game is not started");

	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (!ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);

	Body *nearbody = LuaObject<Body>::CheckFromLua(2);
	float min_dist = luaL_checknumber(l, 3);
	float max_dist = luaL_checknumber(l, 4);

	SystemPath *path = 0;
	double due = -1;
	_unpack_hyperspace_args(l, 5, path, due);

	vector3d newVelocity(nearbody->GetVelocity());
	if (!lua_isnone(l, 6))
		newVelocity = *LuaVector::CheckFromLua(l, 6); // If we have a 6th argument, it better be a vector, otherwise ERROR!!! Hence Check and not Get

	Ship *ship = new Ship(type);
	assert(ship);

	Body *thing = _maybe_wrap_ship_with_cloud(ship, path, due);

	// XXX protect against spawning inside the body
	FrameId newframeId = nearbody->GetFrame();
	Frame *newframe = Frame::GetFrame(newframeId);

	const vector3d newPosition = (MathUtil::RandomPointOnSphere(min_dist, max_dist) * 1000.0) + nearbody->GetPosition();

	// If the frame is rotating and the chosen position is too far, use non-rotating parent.
	// Otherwise the ship will be given a massive initial velocity when it's bumped out of the
	// rotating frame in the next update
	if (newframe->IsRotFrame() && newframe->GetRadius() < newPosition.Length()) {
		assert(newframe->GetParent());
		newframe = Frame::GetFrame(newframe->GetParent());
	}

	thing->SetFrame(newframe->GetId());
	thing->SetPosition(newPosition);
	thing->SetVelocity(newVelocity);
	Pi::game->GetSpace()->AddBody(thing);

	LuaObject<Ship>::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Function: SpawnShipDocked
 *
 * Create a ship and place it inside the given <SpaceStation>.
 *
 * > ship = Space.SpawnShipDocked(type, station)
 *
 * Parameters:
 *
 *   type - the name of the ship
 *
 *   station - the <SpaceStation> to place the ship inside
 *
 * Return:
 *
 *   ship - a <Ship> object for the new ship, or nil if there was no space
 *          inside the station
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_space_spawn_ship_docked(lua_State *l)
{
	if (!Pi::game)
		luaL_error(l, "Game is not started");

	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (!ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);

	SpaceStation *station = LuaObject<SpaceStation>::CheckFromLua(2);

	Ship *ship = new Ship(type);
	assert(ship);

	int port = station->GetFreeDockingPort(ship); // pass in the ship to get a port we fit into
	if (port < 0) {
		delete ship;
		return 0;
	}

	ship->SetFrame(station->GetFrame());
	Pi::game->GetSpace()->AddBody(ship);
	ship->SetDockedWith(station, port);

	LuaObject<Ship>::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Function: SpawnShipParked
 *
 * Create a ship and place it in one of the given <SpaceStation's> parking spots.
 *
 * > ship = Space.SpawnShipParked(type, station)
 *
 * For orbital stations the parking spots are some distance from the door, out
 * of the path of ships entering and leaving the station. For group stations
 * the parking spots are directly above the station, usually some distance
 * away.
 *
 * Parameters:
 *
 *   type - the name of the ship
 *
 *   station - the <SpaceStation> to place the near
 *
 * Return:
 *
 *   ship - a <Ship> object for the new ship, or nil if there was no space
 *          inside the station
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_space_spawn_ship_parked(lua_State *l)
{
	if (!Pi::game)
		luaL_error(l, "Game is not started");

	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (!ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);

	SpaceStation *station = LuaObject<SpaceStation>::CheckFromLua(2);

	int slot;
	if (!station->AllocateStaticSlot(slot))
		return 0;

	Ship *ship = new Ship(type);
	assert(ship);

	const double parkDist = station->GetStationType()->ParkingDistance() - ship->GetPhysRadius();		   // park inside parking radius
	const double parkOffset = (0.5 * station->GetStationType()->ParkingGapSize()) + ship->GetPhysRadius(); // but outside the docking gap

	double xpos = (slot == 0 || slot == 3) ? -parkOffset : parkOffset;
	double zpos = (slot == 0 || slot == 1) ? -parkOffset : parkOffset;
	const vector3d parkPos = station->GetPosition() + station->GetOrient() * vector3d(xpos, parkDist, zpos);

	// orbital stations have Y as axis of rotation
	const matrix3x3d rot = matrix3x3d::RotateX(M_PI / 2) * station->GetOrient();

	ship->SetFrame(station->GetFrame());
	ship->SetVelocity(vector3d(0.0));
	ship->SetPosition(parkPos);
	ship->SetOrient(rot);

	Pi::game->GetSpace()->AddBody(ship);

	ship->AIHoldPosition();

	LuaObject<Ship>::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Function: SpawnShipLanded
 *
 * Create a ship and place it landed on the given <Body>.
 *
 * > ship = Space.SpawnShipLanded(type, body, lat, long)
 *
 * Parameters:
 *
 *   type - the name of the ship
 *
 *   body - the <Body> near which the ship should be spawned
 *
 *   lat - latitude in radians (like in custom body defintions)
 *
 *   long - longitude in radians (like in custom body definitions)
 *
 * Return:
 *
 *   ship - a <Ship> object for the new ship
 *
 * Example:
 *
 * > -- spawn 16km from L.A. when in Sol system
 * > local earth = Space.GetBody(3)
 * > local ship = Space.SpawnShipLanded("viper_police", earth, math.deg2rad(34.06473923), math.deg2rad(-118.1591568))
 *
 * Availability:
 *
 *   July 2013
 *
 * Status:
 *
 *   experimental
 */
static int l_space_spawn_ship_landed(lua_State *l)
{
	if (!Pi::game)
		luaL_error(l, "Game is not started");

	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (!ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);

	Planet *planet = LuaObject<Planet>::CheckFromLua(2);
	if (planet->GetSystemBody()->GetSuperType() != SystemBody::SUPERTYPE_ROCKY_PLANET)
		luaL_error(l, "Body is not a rocky planet");
	float latitude = luaL_checknumber(l, 3);
	float longitude = luaL_checknumber(l, 4);

	Ship *ship = new Ship(type);
	assert(ship);

	Pi::game->GetSpace()->AddBody(ship);
	ship->SetLandedOn(planet, latitude, longitude);

	LuaObject<Ship>::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Function: SpawnShipLandedNear
 *
 * Create a ship and place it on the surface near the given <Body>.
 *
 * > ship = Space.SpawnShipLandedNear(type, body, min, max)
 *
 * Parameters:
 *
 *   type - the name of the ship
 *
 *   body - the <Body> near which the ship should be spawned. It must be on the ground or close to it,
 *          i.e. it must be in the rotating frame of the planetary body.
 *
 *   min - minimum distance from the surface point below the body to place the ship, in Km
 *
 *   max - maximum distance to place the ship
 *
 * Return:
 *
 *   ship - a <Ship> object for the new ship
 *
 * Example:
 *
 * > -- spawn a ship 10km from the player
 * > local ship = Space.SpawnShipLandedNear("viper_police", Game.player, 10, 10)
 *
 * Availability:
 *
 *   July 2013
 *
 * Status:
 *
 *   experimental
 */
static int l_space_spawn_ship_landed_near(lua_State *l)
{
	if (!Pi::game)
		luaL_error(l, "Game is not started");

	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (!ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);

	Body *nearbody = LuaObject<Body>::CheckFromLua(2);
	const float min_dist = luaL_checknumber(l, 3);
	const float max_dist = luaL_checknumber(l, 4);
	if (min_dist > max_dist)
		luaL_error(l, "min_dist must not be larger than max_dist");

	Ship *ship = new Ship(type);
	assert(ship);

	// XXX protect against spawning inside the body
	Frame *newframe = Frame::GetFrame(Frame::GetFrame(nearbody->GetFrame())->GetRotFrame());
	if (!newframe->IsRotFrame())
		luaL_error(l, "Body must be in rotating frame");
	SystemBody *sbody = newframe->GetSystemBody();
	if (sbody->GetSuperType() != SystemBody::SUPERTYPE_ROCKY_PLANET)
		luaL_error(l, "Body is not on a rocky planet");
	if (max_dist > sbody->GetRadius())
		luaL_error(l, "max_dist too large for planet radius");
	// We assume that max_dist is much smaller than the planet radius, i.e. that our area is reasonably flat
	// So, we
	const vector3d up = nearbody->GetPosition().Normalized();
	vector3d x;
	vector3d y;
	// Calculate a orthonormal basis for a horizontal plane. For numerical reasons we do that determining the smallest
	// coordinate and take the cross product with (1,0,0), (0,1,0) or (0,0,1) respectively to calculate the first vector.
	// The second vector is just the cross product of the up-vector and out first vector.
	if (up.x <= up.y && up.x <= up.z) {
		x = vector3d(0.0, up.z, -up.y).Normalized();
		y = vector3d(-up.y * up.y - up.z * up.z, up.x * up.y, up.x * up.z).Normalized();
	} else if (up.y <= up.x && up.y <= up.z) {
		x = vector3d(-up.z, 0.0, up.x).Normalized();
		y = vector3d(up.x * up.y, -up.x * up.x - up.z * up.z, up.y * up.z).Normalized();
	} else {
		x = vector3d(up.y, -up.x, 0.0).Normalized();
		y = vector3d(up.x * up.z, up.y * up.z, -up.x * up.x - up.y * up.y).Normalized();
	}
	Planet *planet = static_cast<Planet *>(newframe->GetBody());
	const double radius = planet->GetSystemBody()->GetRadius();
	const vector3d planar = MathUtil::RandomPointInCircle(min_dist * 1000.0, max_dist * 1000.0);
	vector3d pos = (radius * up + x * planar.x + y * planar.y).Normalized();
	float latitude = atan2(pos.y, sqrt(pos.x * pos.x + pos.z * pos.z));
	float longitude = atan2(pos.x, pos.z);

	Pi::game->GetSpace()->AddBody(ship);
	ship->SetLandedOn(planet, latitude, longitude);

	LuaObject<Ship>::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Function: SpawnCargoNear
 *
 * Create a cargo container and place near the given <Body>
 *
 * > body = Space.SpawnCargoNear(item, body, min, max, lifetime)
 *
 * Parameters:
 *
 *   item - the item to put into the container
 *
 *   body - the <Body> near which the container should be spawned
 *
 *   min - minimum distance from the body to place the container, in m
 *
 *   max - maximum distance to place the container
 *
 *   lifetime - optional time in seconds until self destruct, default is 24h
 *
 * Return:
 *
 *   body - the <Body> object for the requested container
 *
 * Status:
 *
 *   experimental
 */
static int l_space_spawn_cargo_near(lua_State *l)
{
	if (!Pi::game)
		luaL_error(l, "Game is not started");

	LUA_DEBUG_START(l);

	CargoBody *c_body;
	const char *model;

	lua_getfield(l, 1, "model_name");
	if (lua_isstring(l, -1))
		model = lua_tostring(l, -1);
	else
		model = "cargo";

	if (lua_gettop(l) >= 5) {
		float lifetime = lua_tonumber(l, 5);
		c_body = new CargoBody(model, LuaRef(l, 1), lifetime);
	} else {
		c_body = new CargoBody(model, LuaRef(l, 1));
	}
	Body *nearbody = LuaObject<Body>::CheckFromLua(2);
	float min_dist = luaL_checknumber(l, 3);
	float max_dist = luaL_checknumber(l, 4);
	if (min_dist > max_dist)
		luaL_error(l, "min_dist must not be larger than max_dist");

	FrameId frameId = nearbody->GetFrame();
	Frame *frame = Frame::GetFrame(frameId);
	// if the frame is rotating, use non-rotating parent
	if (frame->IsRotFrame()) {
		assert(frame->GetParent());
		frame = Frame::GetFrame(frame->GetParent());
		frameId = frame->GetId();
	}
	c_body->SetFrame(frameId);
	c_body->SetPosition(MathUtil::RandomPointOnSphere(min_dist, max_dist) + nearbody->GetPosition());
	c_body->SetVelocity(_orbital_velocity_random_direction(frame->GetSystemBody(), c_body->GetPosition()));
	Pi::game->GetSpace()->AddBody(c_body);

	LuaObject<Body>::PushToLua(c_body);

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Function: SpawnShipOrbit
 *
 * Create a ship and place it in orbit near the given <Body>.
 *
 * > ship = Space.SpawnShip(type, body, min, max)
 *
 * Parameters:
 *
 *   type - the name of the ship
 *
 *   body - the <Body> near which the ship should be spawned
 *
 *   min - minimum distance from the body to place the ship, in m
 *
 *   max - maximum distance to place the ship
 *
 *
 * Return:
 *
 *   ship - a <Ship> object for the new ship
 *
 * Status:
 *
 *   experimental
 */
static int l_space_spawn_ship_orbit(lua_State *l)
{
	if (!Pi::game)
		luaL_error(l, "Game is not started");

	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (!ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);

	Body *nearbody = LuaObject<Body>::CheckFromLua(2);
	float min_dist = luaL_checknumber(l, 3);
	float max_dist = luaL_checknumber(l, 4);
	if (min_dist > max_dist)
		luaL_error(l, "min_dist must not be larger than max_dist");

	Ship *ship = new Ship(type);
	assert(ship);

	FrameId frameId = nearbody->GetFrame();
	Frame *frame = Frame::GetFrame(frameId);
	// if the frame is rotating, use non-rotating parent
	if (frame->IsRotFrame()) {
		assert(frame->GetParent());
		frame = Frame::GetFrame(frame->GetParent());
		frameId = frame->GetId();
	}
	ship->SetFrame(frameId);
	ship->SetPosition(MathUtil::RandomPointOnSphere(min_dist, max_dist) + nearbody->GetPosition());
	ship->SetVelocity(_orbital_velocity_random_direction(frame->GetSystemBody(), ship->GetPosition()));
	Pi::game->GetSpace()->AddBody(ship);

	LuaObject<Ship>::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Function: GetBody
 *
 * Get the <Body> with the specificed body index.
 *
 * > body = Space.GetBody(index)
 *
 * Parameters:
 *
 *   index - the body index
 *
 * Return:
 *
 *   body - the <Body> object for the requested body, or nil if no such body
 *          exists
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_space_get_body(lua_State *l)
{
	if (!Pi::game) {
		luaL_error(l, "Game is not started");
		return 0;
	}

	int id = luaL_checkinteger(l, 1);

	SystemPath path = Pi::game->GetSpace()->GetStarSystem()->GetPath();
	path.bodyIndex = id;

	Body *b = Pi::game->GetSpace()->FindBodyForPath(&path);
	if (!b) return 0;

	LuaObject<Body>::PushToLua(b);
	return 1;
}

/*
 * Function: GetNumBodies
 *
 * Get the total number of bodies simulated in the current Space
 *
 * bodies = #Space.GetNumBodies()
 *
 * Return:
 *
 *   num - the number of bodies currently existing in Space
 *
 * Availability:
 *
 *   Oct. 2023
 *
 * Status:
 *
 *   stable
 */
static int l_space_get_num_bodies(lua_State *l)
{
	if (!Pi::game) {
		return luaL_error(l, "Game is not started!");
	}

	LuaPush(l, Pi::game->GetSpace()->GetNumBodies());
	return 1;
}

/*
 * Function: GetBodies
 *
 * Get all the <Body> objects that match the specified filter type
 *
 * bodies = Space.GetBodies([type])
 *
 * Parameters:
 *
 *   type - an optional Body classname acting as a filter on the type of the
 *          returned bodies
 *
 * Return:
 *
 *   bodies - an array containing zero or more <Body> objects that matched the
 *            filter
 *
 * Example:
 *
 * > -- get all the ground-based stations
 * > local stations = utils.filter_array(Space.GetBodies("SpaceStation"), function(body)
 * >     return body.type == "STARPORT_SURFACE"
 * > end)
 *
 * Availability:
 *
 *   Oct. 2023
 *
 * Status:
 *
 *   stable
 */
static int l_space_get_bodies(lua_State *l)
{
	PROFILE_SCOPED()

	if (!Pi::game) {
		luaL_error(l, "Game is not started");
		return 0;
	}

	LUA_DEBUG_START(l);

	ObjectType filterBodyType = LuaPull<ObjectType>(l, 1, ObjectType::BODY);
	bool filter = filterBodyType != ObjectType::BODY;

	lua_newtable(l);

	int idx = 1;
	for (Body *b : Pi::game->GetSpace()->GetBodies()) {
		if (filter && !b->IsType(filterBodyType))
			continue;

		lua_pushinteger(l, idx++);
		LuaObject<Body>::PushToLua(b);
		lua_rawset(l, -3);
	}

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Function: GetBodiesNear
 *
 * Get all the <Body> objects within a specified distance from another body
 * that match the specified filter
 *
 * bodies = Space.GetBodiesNear(body, dist, [type])
 *
 * Parameters:
 *
 *   body - the reference body for distance
 *
 *   dist - the maximum distance from the reference body another body can be
 *
 *   type - optional - a PhysicsObjectType enum value
 *          (one of Constants.PhysicsObjectType) acting as a filter on the type
 *          of the returned bodies
 *
 * Return:
 *
 *   bodies - an array containing zero or more <Body> objects that matched the
 *            filter
 *
 * Example:
 *
 * > -- get all stations within 50,000m
 * > local stations = Space.GetBodiesNear(Game.player, 50000, "SPACE_STATION")
 *
 * Availability:
 *
 *   Oct. 2023
 *
 * Status:
 *
 *   stable
 */
static int l_space_get_bodies_near(lua_State *l)
{
	PROFILE_SCOPED()

	if (!Pi::game) {
		luaL_error(l, "Game is not started");
		return 0;
	}

	LUA_DEBUG_START(l);

	Body *body = LuaPull<Body *>(l, 1);
	double dist = LuaPull<double>(l, 2);
	double distSqr = dist * dist;

	ObjectType filterBodyType = LuaPull<ObjectType>(l, 3, ObjectType::BODY);
	bool filter = filterBodyType != ObjectType::BODY;

	lua_newtable(l);

	int idx = 1;
	for (Body *b : Pi::game->GetSpace()->GetBodiesMaybeNear(body, dist)) {
		if (filter && !b->IsType(filterBodyType))
			continue;

		if (b->GetPositionRelTo(body).LengthSqr() > distSqr)
			continue;

		lua_pushinteger(l, idx++);
		LuaObject<Body>::PushToLua(b);
		lua_rawset(l, -3);
	}

	LUA_DEBUG_END(l, 1);

	return 1;
}

static int l_space_dump_frames(lua_State *l)
{
	if (!Pi::game) {
		luaL_error(l, "Game is not started");
		return 0;
	}

	bool details = (lua_gettop(l) >= 1) ? true : false;
	Pi::game->GetSpace()->DebugDumpFrames(details);
	return 0;
}

static int l_space_attr_root_system_body(lua_State *l)
{
	if (!Pi::game) {
		luaL_error(l, "Game is not started");
		return 0;
	}

	LUA_DEBUG_START(l);

	FrameId rootId = Pi::game->GetSpace()->GetRootFrame();
	Frame *root = Frame::GetFrame(rootId);
	LuaObject<SystemBody>::PushToLua(root->GetSystemBody());

	LUA_DEBUG_END(l, 1);

	return 1;
}

void LuaSpace::Register()
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_Reg l_methods[] = {
		{ "SpawnShip", l_space_spawn_ship },
		{ "SpawnShipNear", l_space_spawn_ship_near },
		{ "SpawnShipDocked", l_space_spawn_ship_docked },
		{ "SpawnShipParked", l_space_spawn_ship_parked },
		{ "SpawnShipLanded", l_space_spawn_ship_landed },
		{ "SpawnShipLandedNear", l_space_spawn_ship_landed_near },
		{ "SpawnCargoNear", l_space_spawn_cargo_near },
		{ "SpawnShipOrbit", l_space_spawn_ship_orbit },
		{ "PutShipOnRoute", l_space_put_ship_on_route },
		{ "PutShipIntoOrbit", l_put_ship_into_orbit },

		{ "GetBody", l_space_get_body },
		{ "GetNumBodies", l_space_get_num_bodies },
		{ "GetBodies", l_space_get_bodies },
		{ "GetBodiesNear", l_space_get_bodies_near },

		{ "DbgDumpFrames", l_space_dump_frames },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "rootSystemBody", l_space_attr_root_system_body },
		{ 0, 0 }
	};

	lua_getfield(l, LUA_REGISTRYINDEX, "CoreImports");
	LuaObjectBase::CreateObject(l_methods, l_attrs, 0);
	lua_setfield(l, -2, "Space");
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
