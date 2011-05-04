#include "LuaPlayer.h"
#include "LuaSBodyPath.h"
#include "LuaUtils.h"
#include "Player.h"
#include "Polit.h"

static int l_player_is_player(lua_State *l)
{
    lua_pushboolean(l, true);
    return 1;
}

static void _mission_to_table(lua_State *l, const Mission &m)
{
	LUA_DEBUG_START(l);

	lua_newtable(l);
	pi_lua_table_ro(l);

	pi_lua_settable(l, "ref", m.ref);
	pi_lua_settable(l, "due", m.due);
	pi_lua_settable(l, "reward", (double)m.reward * 0.01);

	lua_pushstring(l, "type");
	lua_pushstring(l, m.type.c_str());
	lua_rawset(l, -3);

	lua_pushstring(l, "client");
	lua_pushstring(l, m.client.c_str());
	lua_rawset(l, -3);

	lua_pushstring(l, "location");
	LuaSBodyPath::PushToLuaGC(new SBodyPath(m.location));
	lua_rawset(l, -3);

	lua_pushstring(l, "status");
	switch (m.status) {
		case Mission::COMPLETED:
			lua_pushstring(l, "completed");
			break;

		case Mission::FAILED:
			lua_pushstring(l, "failed");
			break;

		case Mission::ACTIVE:
		default:
			lua_pushstring(l, "active");
			break;
	}
	lua_rawset(l, -3);

	LUA_DEBUG_END(l, 1);
}

static void _table_to_mission(lua_State *l, Mission &m, bool create)
{
	LUA_DEBUG_START(l);

	// XXX sucky. should report errors that tell you which field is broken

	lua_getfield(l, -1, "due");
	if (create || !lua_isnil(l, -1))
		m.due = luaL_checknumber(l, -1);
	lua_pop(l, 1);

	lua_getfield(l, -1, "reward");
	if (create || !lua_isnil(l, -1))
		m.reward = (Sint64)(luaL_checknumber(l, -1) * 100.0);
	lua_pop(l, 1);

	lua_getfield(l, -1, "type");
	if (create || !lua_isnil(l, -1))
		m.type = luaL_checkstring(l, -1);
	lua_pop(l, 1);

	lua_getfield(l, -1, "client");
	if (create || !lua_isnil(l, -1))
		m.client = luaL_checkstring(l, -1);
	lua_pop(l, 1);

	lua_getfield(l, -1, "location");
	if (create || !lua_isnil(l, -1)) {
		SBodyPath *sbody = LuaSBodyPath::GetFromLua(-1);
		m.location = *sbody;
	}
	lua_pop(l, 1);

	lua_getfield(l, -1, "status");
	if (lua_isnil(l, -1)) {
		if (create)
			m.status = Mission::ACTIVE;
	}
	else {
		std::string status = luaL_checkstring(l, -1);

		if (status == "completed")
			m.status = Mission::COMPLETED;
		else if (status == "failed")
			m.status = Mission::FAILED;
		else if (status == "active")
			m.status = Mission::ACTIVE;
		else
			luaL_error(l, "field 'status' has unknown value '%s'", status.c_str());
	}
	
	lua_pop(l, 2);

	LUA_DEBUG_END(l, -1);
}

static int l_player_add_mission(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);

	Mission m;
	_table_to_mission(l, m, true);

	int ref = p->missions.Add(m);
	lua_pushinteger(l, ref);

	return 1;
}

static int l_player_update_mission(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	int ref = luaL_checkinteger(l, 2);

	const Mission *m = p->missions.Get(ref);
	if (!m)
		luaL_error(l, "mission with ref %d not found", ref);
	
	Mission upm = *m;
	_table_to_mission(l, upm, false);

	p->missions.Update(ref, upm);

	return 0;
}

static int l_player_remove_mission(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	int ref = luaL_checkinteger(l, 2);
	p->missions.Remove(ref);
	return 0;
}

static int l_player_get_mission(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	int ref = luaL_checkinteger(l, 2);
	const Mission *m = p->missions.Get(ref);
	if (!m)
		lua_pushnil(l);
	else
		_mission_to_table(l, *m);
	return 1;
}

static int l_player_get_money(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	lua_pushnumber(l, p->GetMoney()*0.01);
	return 1;
} 

static int l_player_set_money(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	float m = luaL_checknumber(l, 2);
	p->SetMoney((Sint64)(m*100.0));
	return 0;
} 

static int l_player_add_money(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	float a = luaL_checknumber(l, 2);
	Sint64 m = p->GetMoney() + (Sint64)(a*100.0);
	p->SetMoney(m);
	lua_pushnumber(l, m*0.01);
	return 1;
}

// XXX this most certainly does not belong here. it will be removed when the
// entire polit/crime system is evicted to lua where it belongs
static int l_player_add_crime(lua_State *l)
{
	Sint64 crimeBitset = luaL_checkinteger(l, 1);
	double fine = (Sint64)(luaL_checknumber(l, 2) * 100.0);
	Polit::AddCrime(crimeBitset, fine);
	return 0;
}

static bool promotion_test(DeleteEmitter *o)
{
	return dynamic_cast<Player*>(o);
}

template <> const char *LuaObject<Player>::s_type = "Player";

template <> void LuaObject<Player>::RegisterClass()
{
	static const char *l_parent = "Ship";

	static const luaL_reg l_methods[] = {
		{ "IsPlayer", l_player_is_player },

		{ "AddMission",    l_player_add_mission    },
		{ "UpdateMission", l_player_update_mission },
		{ "RemoveMission", l_player_remove_mission },
		{ "GetMission",    l_player_get_mission    },

		{ "GetMoney", l_player_get_money },
		{ "SetMoney", l_player_set_money },
		{ "AddMoney", l_player_add_money },

		{ "AddCrime",      l_player_add_crime      },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, NULL, NULL);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
