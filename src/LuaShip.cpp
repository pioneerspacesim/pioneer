#include "LuaShip.h"
#include "LuaSpaceStation.h"
#include "LuaSBodyPath.h"
#include "LuaShipType.h"
#include "LuaUtils.h"
#include "Ship.h"
#include "SpaceStation.h"
#include "ShipType.h"

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
	
	if (lua_isnil(l, 2)) {
		// no attributes:
		// - 0-10AU from star surface
		// - basic fitout: 1mw laser, standard missiles, atmos shielding
		assert(0);
	}

	if (!lua_istable(l, 2))
		luaL_typerror(l, 2, lua_typename(l, LUA_TTABLE);

	// attribute table
	//
	//  position: table of attributes defining a spawn location. first
	//            element is a string that says how to interpret the rest of
	//            the args:
	//      { "system", x.x, y.y }
	//          random position between x and y AU from primary star
	//      { "near", body, x.x, y.y }
	//          random position between x and y km from body
	//      { "docked", starport }
	//          docked with starport
	//      { "parked", starport }
	//          parked near starport (in orbit over starport or near station entrance
	//      defaults to { "system", 0.0, 10.0 }
	//  
	//  power: single number from 0.0 to 1.0, where 0 is unarmed and 1.0 is armed to the teeth.
	//         adds weaponry and other equipment to bring the ship up to the desired powered level
	//         if not specified the ship receives no equipment automatically
	//
	//  due: single number N. spawns the ship N seconds from now. if N is 0,
	//       spawns a hyperspace cloud remnant and has the ship emerge from
	//       it. if N > 0, takes an additional SBodyPath argument and creates
	//       an arrival cloud with that system as the source. the ship will
	//       emerge at the specified time
	//       if not specified the ship will simply "appear" with no hyperspace
	//       cloud

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
