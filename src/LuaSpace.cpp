#include "LuaSpace.h"
#include "LuaManager.h"
#include "LuaUtils.h"
#include "LuaShip.h"
#include "LuaSystemPath.h"
#include "LuaBody.h"
#include "LuaSpaceStation.h"
#include "LuaStar.h"
#include "LuaPlanet.h"
#include "Space.h"
#include "Ship.h"
#include "HyperspaceCloud.h"
#include "Pi.h"
#include "SpaceStation.h"
#include "Player.h"

/*
 * Interface: Space
 *
 * Various functions to create and find objects in the current physics space.
 */

static void _unpack_hyperspace_args(lua_State *l, int index, SystemPath* &path, double &due)
{
	if (lua_isnone(l, index)) return;

	if (!lua_istable(l, index))
		luaL_typerror(l, index, lua_typename(l, LUA_TTABLE));
	
	LUA_DEBUG_START(l);

	lua_pushinteger(l, 1);
	lua_gettable(l, index);
	if (!(path = LuaSystemPath::CheckFromLua(-1)))
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

static Body *_maybe_wrap_ship_with_cloud(Ship *ship, SystemPath *path, double due)
{
	if (!path) return ship;

	HyperspaceCloud *cloud = new HyperspaceCloud(ship, due, true);
	ship->SetHyperspaceDest(path);

	return cloud;
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
 * > local ship = Ship.Spawn("Eagle Long Range Fighter, 5, 6)
 *
 * > -- spawn a ship in the ~11AU hyperspace area and make it appear that it
 * > -- came from Sol and will arrive in ten minutes
 * > local ship = Ship.Spawn(
 * >     "Flowerfairy Heavy Trader", 9, 11,
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
	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);
	
	float min_dist = luaL_checknumber(l, 2);
	float max_dist = luaL_checknumber(l, 3);

	SystemPath *path = NULL;
	double due = -1;
	_unpack_hyperspace_args(l, 4, path, due);

	Ship *ship = new Ship(type);
	assert(ship);

	Body *thing = _maybe_wrap_ship_with_cloud(ship, path, due);

	// XXX protect against spawning inside the body
	thing->SetFrame(Space::rootFrame);
	if (path == NULL)
		thing->SetPosition(Space::GetRandomPosition(min_dist, max_dist)*AU);
	else
		thing->SetPosition(Space::GetPositionAfterHyperspace(path, &(Pi::currentSystem->GetPath())));
	thing->SetVelocity(vector3d(0,0,0));
	Space::AddBody(thing);

	LuaShip::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

/*
 * Function: SpawnShipNear
 *
 * Create a ship and place it in space near the given <Body>.
 *
 * > ship = Space.SpawnShip(type, body, min, max, hyperspace)
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
 * Return:
 *
 *   ship - a <Ship> object for the new ship
 *
 * Example:
 *
 * > -- spawn a ship 10km from the player
 * > local ship = Ship.SpawnNear("Viper Police Craft", Game.player, 10, 10)
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
	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);
	
	Body *nearbody = LuaBody::GetFromLua(2);
	float min_dist = luaL_checknumber(l, 3);
	float max_dist = luaL_checknumber(l, 4);

	SystemPath *path = NULL;
	double due = -1;
	_unpack_hyperspace_args(l, 5, path, due);

	Ship *ship = new Ship(type);
	assert(ship);

	Body *thing = _maybe_wrap_ship_with_cloud(ship, path, due);

	// XXX protect against spawning inside the body
	thing->SetFrame(nearbody->GetFrame());
	thing->SetPosition((Space::GetRandomPosition(min_dist, max_dist)* 1000.0) + nearbody->GetPosition());
	thing->SetVelocity(vector3d(0,0,0));
	Space::AddBody(thing);

	LuaShip::PushToLua(ship);

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
	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);
	
	SpaceStation *station = LuaSpaceStation::GetFromLua(2);

	int port = station->GetFreeDockingPort();
	if (port < 0)
		return 0;
	
	Ship *ship = new Ship(type);
	assert(ship);

	ship->SetFrame(station->GetFrame());
	Space::AddBody(ship);
	ship->SetDockedWith(station, port);

	station->CreateBB();

	LuaShip::PushToLua(ship);

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
	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);
	
	SpaceStation *station = LuaSpaceStation::GetFromLua(2);

	int slot;
	if (!station->AllocateStaticSlot(slot))
		return 0;

	Ship *ship = new Ship(type);
	assert(ship);

	vector3d pos, vel;
	matrix4x4d rot = matrix4x4d::Identity();

	if (station->GetSBody()->type == SBody::TYPE_STARPORT_SURFACE) {
		vel = vector3d(0.0);

		// XXX on tiny planets eg asteroids force this to be larger so the
		// are out of the docking path
		pos = station->GetPosition() * 1.1;
		station->GetRotMatrix(rot);

		vector3d axis1, axis2;

		axis1 = pos.Cross(vector3d(0.0,1.0,0.0));
		axis2 = pos.Cross(axis1);

		double ang = atan((140 + ship->GetLmrCollMesh()->GetBoundingRadius()) / pos.Length());
		if (slot<2) ang = -ang;

		vector3d axis = (slot == 0 || slot == 3) ? axis1 : axis2;

		pos.ArbRotate(axis, ang);
	}

	else {
		double dist = 100 + ship->GetLmrCollMesh()->GetBoundingRadius();
		double xpos = (slot == 0 || slot == 3) ? -dist : dist;
		double zpos = (slot == 0 || slot == 1) ? -dist : dist;

		pos = vector3d(xpos,5000,zpos);
		vel = vector3d(0.0);
		rot.RotateX(M_PI/2);
	}

	ship->SetFrame(station->GetFrame());

	ship->SetVelocity(vel);
	ship->SetPosition(pos);
	ship->SetRotMatrix(rot);

	Space::AddBody(ship);

	ship->AIHoldPosition();
	
	LuaShip::PushToLua(ship);

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
	int id = luaL_checkinteger(l, 1);

	SystemPath path = Pi::currentSystem->GetPath();
	path.bodyIndex = id;

	Body *b = Space::FindBodyForPath(&path);
	if (!b) return 0;

	LuaBody::PushToLua(b);
	return 1;
}

/*
 * Function: GetBodies
 *
 * Get all the <Body> objects that match the specified filter
 *
 * bodies = Space.GetBodies(filter)
 *
 * Parameters:
 *
 *   filter - an option function. If specificed the function will be called
 *            once for each body with the <Body> object as the only parameter.
 *            If the filter function returns true then the <Body> will be
 *            included in the array returned by <GetBodies>, otherwise it will
 *            be omitted. If no filter function is specified then all bodies
 *            are returned.
 *
 * Return:
 *
 *   bodies - an array containing zero or more <Body> objects that matched the
 *            filter
 *
 * Example:
 *
 * > -- get all the ground-based stations
 * > local stations = Space.GetBodies(function (body)
 * >     return body.type == "STARPORT_SURFACE"
 * > end)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_space_get_bodies(lua_State *l)
{
	LUA_DEBUG_START(l);

	bool filter = false;
	if (lua_gettop(l) >= 1) {
		if (!lua_isfunction(l, 1))
			luaL_typerror(l, 1, lua_typename(l, LUA_TFUNCTION));
		filter = true;
	}

	lua_newtable(l);
	pi_lua_table_ro(l);

	for (std::list<Body*>::iterator i = Space::bodies.begin(); i != Space::bodies.end(); i++) {
		Body *b = *i;

		if (filter) {
			lua_pushvalue(l, 1);
			LuaBody::PushToLua(b);
			if (int ret = lua_pcall(l, 1, 1, 0)) {
				const char *errmsg;
				if (ret == LUA_ERRRUN)
					errmsg = lua_tostring(l, -1);
				else if (ret == LUA_ERRMEM)
					errmsg = "memory allocation failure";
				else if (ret == LUA_ERRERR)
					errmsg = "error in error handler function";
				luaL_error(l, "Error in filter function: %s", errmsg);
			}
			if (!lua_toboolean(l, -1)) {
				lua_pop(l, 1);
				continue;
			}
			lua_pop(l, 1);
		}

		lua_pushinteger(l, lua_objlen(l, -1)+1);
		LuaBody::PushToLua(b);
		lua_rawset(l, -3);
    }

	LUA_DEBUG_END(l, 1);

	return 1;
}

void LuaSpace::Register()
{
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "SpawnShip",       l_space_spawn_ship        },
		{ "SpawnShipNear",   l_space_spawn_ship_near   },
		{ "SpawnShipDocked", l_space_spawn_ship_docked },
		{ "SpawnShipParked", l_space_spawn_ship_parked },

		{ "GetBody",   l_space_get_body   },
		{ "GetBodies", l_space_get_bodies },
		{ 0, 0 }
	};

	luaL_register(l, "Space", methods);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}
