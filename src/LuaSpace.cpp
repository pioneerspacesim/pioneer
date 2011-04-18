#include "LuaSpace.h"
#include "LuaManager.h"
#include "LuaUtils.h"
#include "LuaShip.h"
#include "LuaSBodyPath.h"
#include "LuaBody.h"
#include "LuaSpaceStation.h"
#include "Space.h"
#include "Ship.h"
#include "HyperspaceCloud.h"
#include "Pi.h"
#include "SpaceStation.h"
#include "Player.h"

static void _unpack_hyperspace_args(lua_State *l, int index, SBodyPath* &path, double &due)
{
	if (lua_isnone(l, index)) return;

	if (!lua_istable(l, index))
		luaL_typerror(l, index, lua_typename(l, LUA_TTABLE));
	
	LUA_DEBUG_START(l);

	lua_pushinteger(l, 1);
	lua_gettable(l, index);
	if (!(path = LuaSBodyPath::CheckFromLua(-1)))
		luaL_error(l, "bad value for hyperspace path at position 1 (SBodyPath expected, got %s)", luaL_typename(l, -1));
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

static Body *_maybe_wrap_ship_with_cloud(Ship *ship, SBodyPath *path, double due)
{
	if (!path) return ship;

	HyperspaceCloud *cloud = new HyperspaceCloud(ship, due, true);
	ship->SetHyperspaceTarget(path);

	return cloud;
}

static int l_space_spawn_ship(lua_State *l)
{
	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);
	
	float min_dist = luaL_checknumber(l, 2);
	float max_dist = luaL_checknumber(l, 3);

	SBodyPath *path = NULL;
	double due = -1;
	_unpack_hyperspace_args(l, 4, path, due);

	Ship *ship = new Ship(type);
	assert(ship);

	Body *thing = _maybe_wrap_ship_with_cloud(ship, path, due);

	// XXX protect against spawning inside the body
	float longitude = Pi::rng.Double(-M_PI,M_PI);
	float latitude = Pi::rng.Double(-M_PI,M_PI);

	float dist = (min_dist + Pi::rng.Double(max_dist-min_dist));
	vector3d pos = vector3d(sin(longitude)*cos(latitude), sin(latitude), cos(longitude)*cos(latitude));

	thing->SetFrame(Space::rootFrame);
	thing->SetPosition(pos * dist * AU);
	thing->SetVelocity(vector3d(0,0,0));
	Space::AddBody(thing);

	LuaShip::PushToLua(ship);

	Pi::player->SetNavTarget(thing);

	LUA_DEBUG_END(l, 1);

	return 1;
}

static int l_space_spawn_ship_near(lua_State *l)
{
	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);
	
	Body *near = LuaBody::GetFromLua(2);
	float min_dist = luaL_checknumber(l, 3);
	float max_dist = luaL_checknumber(l, 4);

	SBodyPath *path = NULL;
	double due = -1;
	_unpack_hyperspace_args(l, 5, path, due);

	Ship *ship = new Ship(type);
	assert(ship);

	Body *thing = _maybe_wrap_ship_with_cloud(ship, path, due);

	// XXX protect against spawning inside the body
	float longitude = Pi::rng.Double(-M_PI,M_PI);
	float latitude = Pi::rng.Double(-M_PI,M_PI);

	float dist = (min_dist + Pi::rng.Double(max_dist-min_dist));
	vector3d pos = vector3d(sin(longitude)*cos(latitude), sin(latitude), cos(longitude)*cos(latitude));

	thing->SetFrame(near->GetFrame());
	thing->SetPosition((pos * dist * 1000.0) + near->GetPosition());
	thing->SetVelocity(vector3d(0,0,0));
	Space::AddBody(thing);

	LuaShip::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

static int l_space_spawn_ship_docked(lua_State *l)
{
	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);
	
	SpaceStation *station = LuaSpaceStation::GetFromLua(2);

	int port = station->GetFreeDockingPort();
	if (port < 0)
		luaL_error(l, "No free docking ports in station");
	
	Ship *ship = new Ship(type);
	assert(ship);

	ship->SetFrame(station->GetFrame());
	Space::AddBody(ship);
	ship->SetDockedWith(station, port);

	LuaShip::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

static int l_space_spawn_ship_parked(lua_State *l)
{
	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);
	
	SpaceStation *station = LuaSpaceStation::GetFromLua(2);

	int slot;
	if (!station->AllocateStaticSlot(slot))
		luaL_error(l, "No free parking slots near station");

	Ship *ship = new Ship(type);
	assert(ship);

	vector3d pos, vel;
	matrix4x4d rot = matrix4x4d::Identity();

	if (station->GetSBody()->type == SBody::TYPE_STARPORT_SURFACE) {
		vel = vector3d(0.0);

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

	ship->AIHoldPosition(station);
	
	LuaShip::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

void LuaSpace::Register()
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	LUA_DEBUG_START(l);

	static const luaL_reg methods[] = {
		{ "SpawnShip",       l_space_spawn_ship        },
		{ "SpawnShipNear",   l_space_spawn_ship_near   },
		{ "SpawnShipDocked", l_space_spawn_ship_docked },
		{ "SpawnShipParked", l_space_spawn_ship_parked },
		{ 0, 0 }
	};

	luaL_register(l, "Space", methods);

	LUA_DEBUG_END(l, 0);
}
