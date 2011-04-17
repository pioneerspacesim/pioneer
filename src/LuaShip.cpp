#include "LuaShip.h"
#include "LuaSpaceStation.h"
#include "LuaSBodyPath.h"
#include "LuaShipType.h"
#include "LuaBody.h"
#include "LuaUtils.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "ShipType.h"
#include "Space.h"
#include "Pi.h"
#include "Player.h"

static int l_ship_get_stats(lua_State *l)
{
	LUA_DEBUG_START(l);

	Ship *s = LuaShip::GetFromLua(1);
	const shipstats_t *stats = s->CalcStats();
	
	lua_newtable(l);
	pi_lua_settable(l, "max_capacity",         stats->max_capacity);
	pi_lua_settable(l, "used_capacity",        stats->used_capacity);
	pi_lua_settable(l, "used_cargo",           stats->used_cargo);
	pi_lua_settable(l, "free_capacity",        stats->free_capacity);
	pi_lua_settable(l, "total_mass",           stats->total_mass);
	pi_lua_settable(l, "hull_mass_left",       stats->hull_mass_left);
	pi_lua_settable(l, "hyperspace_range",     stats->hyperspace_range);
	pi_lua_settable(l, "hyperspace_range_max", stats->hyperspace_range_max);
	pi_lua_settable(l, "shield_mass",          stats->shield_mass);
	pi_lua_settable(l, "shield_mass_left",     stats->shield_mass_left);

	LUA_DEBUG_END(l, 1);

	return 1;
}

static int l_ship_get_money(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	lua_pushnumber(l, s->GetMoney()*0.01);
	return 1;
} 

static int l_ship_set_money(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	float m = luaL_checknumber(l, 2);
	s->SetMoney((Sint64)(m*100.0));
	return 0;
} 

static int l_ship_add_money(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	float a = luaL_checknumber(l, 2);
	Sint64 m = s->GetMoney() + (Sint64)(a*100.0);
	s->SetMoney(m);
	lua_pushnumber(l, m*0.01);
	return 1;
}

static int l_ship_get_docked_with(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	SpaceStation *station = s->GetDockedWith();
	if (!station) return 0;
	LuaSpaceStation::PushToLua(station);
	return 1;
}

static int l_ship_ai_do_kill(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	Ship *target = LuaShip::GetFromLua(2);
	s->AIKill(target);
	return 0;
}

static int l_ship_ai_do_flyto(lua_State *l)
{
	/*
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIDoFlyTo(target);
	*/
	return 0;
}

static int l_ship_ai_do_dock(lua_State *l)
{
	Ship *s = LuaShip::GetFromLua(1);
	SpaceStation *target = LuaSpaceStation::GetFromLua(2);
	s->AIDock(target);
	return 0;
}

static int l_ship_ai_do_loworbit(lua_State *l)
{
	/*
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIOrbit(target, 1.1);
	*/
	return 0;
}

static int l_ship_ai_do_mediumorbit(lua_State *l)
{
	/*
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIOrbit(target, 2.0);
	*/
	return 0;
}

static int l_ship_ai_do_highorbit(lua_State *l)
{
	/*
	Ship *s = LuaShip::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
	s->AIOrbit(target, 5.0);
	*/
	return 0;
}

static int l_ship_ai_do_journey(lua_State *l)
{
	/*
	
	XXX is this even required anymore?

	Ship *s = LuaShip::GetFromLua(1);
	SBodyPath *dest = LuaSBodyPath::GetFromLua(2);
	s->AIJourney(dest);
	*/
	return 0;
}

static int l_ship_get_ship_types(lua_State *l)
{
	LUA_DEBUG_START(l);

	ShipType::Tag tag = ShipType::TAG_NONE;

	if (lua_gettop(l) >= 1)
		tag = static_cast<ShipType::Tag>(luaL_checkinteger(l, 1));
	
	if (tag < 0 || tag >= ShipType::TAG_MAX)
		luaL_error(l, "Unknown ship tag %d", tag);

	// remove this if there's a time when the ship list can change mid-game
	lua_getfield(l, LUA_REGISTRYINDEX, "PiShipTypeTable");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiShipTypeTable");
	}

	if (tag == ShipType::TAG_NONE)
		lua_getfield(l, -1, "_all");
	else {
		lua_pushvalue(l, 1);
		lua_gettable(l, -2);
	}
	
	if (lua_istable(l, -1)) {
		lua_remove(l, -2);
		LUA_DEBUG_END(l, 1);
		return 1;
	}
	lua_pop(l, 1);

	lua_newtable(l);
	pi_lua_table_ro(l);
	
	if (tag == ShipType::TAG_NONE)
		lua_pushinteger(l, tag);
	else
		lua_pushstring(l, "_all");
	lua_pushvalue(l, -2);
	lua_settable(l, -4);

	lua_remove(l, -2);
		
	for (std::map<ShipType::Type,ShipType>::iterator i = ShipType::types.begin(); i != ShipType::types.end(); i++)
	{
		ShipType *st = &((*i).second);
		if (tag == ShipType::TAG_NONE || tag == st->tag) {
			lua_pushstring(l, (*i).first.c_str());
			LuaShipType::PushToLua(st);
			lua_rawset(l, -3);
		}
	}

	LUA_DEBUG_END(l, 1);

	return 1;
}

static int l_ship_spawn(lua_State *l)
{
	LUA_DEBUG_START(l);

	const char *type = luaL_checkstring(l, 1);
	if (! ShipType::Get(type))
		luaL_error(l, "Unknown ship type '%s'", type);
	
	if (lua_isnoneornil(l, 2))
		lua_newtable(l);
	else if (!lua_istable(l, 2))
		luaL_typerror(l, 2, lua_typename(l, LUA_TTABLE));
	else
		lua_pushvalue(l, 2);
	
	// attribute table
	//
	//  location: table of attributes defining a spawn location. first
	//            element is a string that says how to interpret the rest of
	//            the args:
	//      { "system", x.x, y.y }
	//          random location between x and y AU from primary star
	//      { "near", body, x.x, y.y }
	//          random location between x and y km from body
	//      { "docked", station }
	//          docked with station
	//      { "parked", station }
	//          parked near station (in orbit over ground station or near orbital station entrance)
	//      defaults to { "system", 0.0, 10.0 }
	//  
	//  power: single number from 0.0 to 1.0, where 0 is unarmed and 1.0 is armed to the teeth.
	//         adds weaponry and other equipment to bring the ship up to the desired powered level
	//         if not specified the ship receives no equipment automatically
	//
	//  hyperspace: table with two elements. first is a SBodyPath for the
	//              system the ship left from. the second is single number
	//              indicating the spawn time in secpnds from now
	
	enum { _SYSTEM, _NEAR, _DOCKED, _PARKED } location = _SYSTEM;
	Body *body = NULL;
	SpaceStation *station = NULL;
	double min_dist = 0.0, max_dist = 10.0;
	double power = -1;
	SBodyPath *path = NULL;
	double due = -1;
	
	lua_getfield(l, -1, "location");
	if (!lua_isnil(l, -1)) {
		if (!lua_istable(l, -1))
			luaL_error(l, "bad value for 'location' (%s expected, got %s)", lua_typename(l, LUA_TTABLE), luaL_typename(l, -1));

		int i = 0;
		lua_pushinteger(l, ++i);
		lua_gettable(l, -2);
		if (!lua_isstring(l, -1))
			luaL_error(l, "bad value for location type at position %d (%s expected, got %s)", i, lua_typename(l, LUA_TTABLE), luaL_typename(l, -1));
		std::string locstr = lua_tostring(l, -1);
		lua_pop(l, 1);

		if (locstr == "system" || locstr == "near") {
			if (locstr == "near") {
				location = _NEAR;

				lua_pushinteger(l, ++i);
				lua_gettable(l, -2);
				if (!(body = LuaBody::CheckFromLua(-1)))
					luaL_error(l, "bad value for location 'near' system body at position %d (Body expected, got %s)", i, luaL_typename(l, -1));
				lua_pop(l, 1);

				// fall through
			}

			lua_pushinteger(l, ++i);
			lua_gettable(l, -2);
			if (!lua_isnumber(l, -1))
				luaL_error(l, "bad value for location '%s' at position %d (%s expected, got %s)", locstr.c_str(), i, lua_typename(l, LUA_TNUMBER), luaL_typename(l, -1));
			min_dist = lua_tonumber(l, -1);
			if (min_dist < 0)
				luaL_error(l, "bad value for location '%s' minimum distance at position %d", locstr.c_str(), i);
			lua_pop(l, 1);

			lua_pushinteger(l, ++i);
			lua_gettable(l, -2);
			if (!lua_isnumber(l, -1))
				luaL_error(l, "bad value for location '%s' at position %d (%s expected, got %s)", locstr.c_str(), i, lua_typename(l, LUA_TNUMBER), luaL_typename(l, -1));
			max_dist = lua_tonumber(l, -1);
			if (max_dist <= min_dist)
				luaL_error(l, "bad value for location '%s' maximum distance at position %d", locstr.c_str(), i);
			lua_pop(l, 1);
		}

		else if (locstr == "docked" || locstr == "parked") {
			location = locstr == "docked" ? _DOCKED : _PARKED;

			lua_pushinteger(l, ++i);
			lua_gettable(l, -2);
			if (!(station = LuaSpaceStation::CheckFromLua(-1)))
				luaL_error(l, "bad value for location '%s' at position %d (SpaceStation expected, got %s)", locstr.c_str(), i, luaL_typename(l, -1));
			lua_pop(l, 1);
		}

		else
			luaL_error(l, "unknown location type '%s'", locstr.c_str());
	}
	lua_pop(l, 1);
	
	lua_getfield(l, -1, "power");
	if (!lua_isnil(l, -1)) {
		if (!lua_isnumber(l, -1))
			luaL_error(l, "bad value for 'power' (%s expected, got %s)", lua_typename(l, LUA_TNUMBER), luaL_typename(l, -1));
		power = lua_tonumber(l, -1);
		if (power < 0 || power > 1)
			luaL_error(l, "bad value for 'power' (out of range, must 0.0 - 1.0)");
		lua_pop(l, 1);
	}
	lua_pop(l, 1);

	if (location == _DOCKED || location == _PARKED) {
		lua_getfield(l, -1, "hyperspace");
		if (!lua_isnil(l, -1)) {
			if (!lua_istable(l, -1))
				luaL_error(l, "bad value for 'hyperspace' (%s expected, got %s)", lua_typename(l, LUA_TTABLE), luaL_typename(l, -1));

			lua_pushinteger(l, 1);
			lua_gettable(l, -2);
			if (!(path = LuaSBodyPath::CheckFromLua(-1)))
				luaL_error(l, "bad value for hyperspace path at position 1 (SBodyPath expected, got %s)", luaL_typename(l, -1));
			lua_pop(l, 1);

			lua_pushinteger(l, 2);
			lua_gettable(l, -2);
			if (!(lua_isnumber(l, -1)))
				luaL_error(l, "bad value for hyperspace exit time at position 2 (%s expected, got %s)", lua_typename(l, LUA_TNUMBER), luaL_typename(l, -1));
			due = lua_tonumber(l, -1);
			if (due < 0)
				luaL_error(l, "bad value for hyperspace exit time at position 2 (must be >= 0)");
			lua_pop(l, 1);
		}
		lua_pop(l, 1);
	}

	// spawn it!
	Ship *ship = new Ship(type);
	assert(ship);

	if (power >= 0) {
		// XXX fill it with stuff
	}

	Body *thing;
	if (path) {
		// XXX make a cloud, wrap it around the ship
	}
	else
		thing = ship;

	if (location == _SYSTEM || location == _NEAR) {
		// XXX protect against spawning inside the body

		float longitude = Pi::rng.Double(-M_PI,M_PI);
		float latitude = Pi::rng.Double(-M_PI,M_PI);

		float dist = (min_dist + Pi::rng.Double(max_dist-min_dist));
		vector3d pos = vector3d(sin(longitude)*cos(latitude), sin(latitude), cos(longitude)*cos(latitude));

		if (location == _SYSTEM) {
			pos *= (dist*AU);
			thing->SetFrame(Space::rootFrame);
		}
		else {
			pos *= (dist*1000.0);
			pos += body->GetPosition();
			thing->SetFrame(body->GetFrame());
		}

		thing->SetPosition(pos);
		thing->SetVelocity(vector3d(0,0,0));
		Space::AddBody(thing);
	}

	else if (location == _DOCKED) {
		int port = station->GetFreeDockingPort();
		if (port < 0) {
			delete ship;
			luaL_error(l, "no free docking ports in station");
		}

		ship->SetFrame(station->GetFrame());
		Space::AddBody(ship);
		ship->SetDockedWith(station, port);
	}

	else if (location == _PARKED) {
		int slot;
		if (!station->AllocateStaticSlot(slot)) {
			delete ship;
			luaL_error(l, "no free parking slots near station");
		}

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
	}

	else
		// can't happen
		assert(0);
	
	Pi::player->SetCombatTarget(ship);

	LuaShip::PushToLua(ship);

	LUA_DEBUG_END(l, 1);

	return 1;
}

template <> const char *LuaObject<Ship>::s_type = "Ship";

template <> void LuaObject<Ship>::RegisterClass()
{
	static const char *l_inherit = "Body";

	static const luaL_reg l_methods[] = {
		{ "GetStats", l_ship_get_stats },

		{ "GetMoney", l_ship_get_money },
		{ "SetMoney", l_ship_set_money },
		{ "AddMoney", l_ship_add_money },

		{ "GetDockedWith", l_ship_get_docked_with },

		{ "AIDoKill",        l_ship_ai_do_kill        },
		{ "AIDoFlyto",       l_ship_ai_do_flyto       },
		{ "AIDoDock",        l_ship_ai_do_dock        },
		{ "AIDoLoworbit",    l_ship_ai_do_loworbit    },
		{ "AIDoMediumorbit", l_ship_ai_do_mediumorbit },
		{ "AIDoHighorbit",   l_ship_ai_do_highorbit   },
		{ "AIDoJourney",     l_ship_ai_do_journey     },

		{ "GetShipTypes",    l_ship_get_ship_types    },

		{ "Spawn",           l_ship_spawn             },

		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_inherit, l_methods, NULL);
}
