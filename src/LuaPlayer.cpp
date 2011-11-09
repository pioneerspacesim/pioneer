#include "LuaPlayer.h"
#include "LuaSystemPath.h"
#include "LuaBody.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "Player.h"
#include "Polit.h"

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

static void _mission_to_table(lua_State *l, const Mission &m)
{
	LUA_DEBUG_START(l);

	lua_newtable(l);
	pi_lua_table_ro(l);

	pi_lua_settable(l, "ref", m.ref);
	pi_lua_settable(l, "due", m.due);
	pi_lua_settable(l, "reward", double(m.reward) * 0.01);

	lua_pushstring(l, "type");
	lua_pushstring(l, m.type.c_str());
	lua_rawset(l, -3);

	lua_pushstring(l, "client");
	lua_pushstring(l, m.client.c_str());
	lua_rawset(l, -3);

	lua_pushstring(l, "location");
	LuaSystemPath::PushToLuaGC(new SystemPath(m.location));
	lua_rawset(l, -3);

	lua_pushstring(l, "status");
	lua_pushstring(l, LuaConstants::GetConstantString(l, "MissionStatus", m.status));
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
		m.reward = Sint64(luaL_checknumber(l, -1) * 100.0);
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
		SystemPath *sbody = LuaSystemPath::GetFromLua(-1);
		m.location = *sbody;
	}
	lua_pop(l, 1);

	lua_getfield(l, -1, "status");
	if (lua_isnil(l, -1)) {
		if (create)
			m.status = Mission::ACTIVE;
	}
	else
		m.status = static_cast<Mission::MissionState>(LuaConstants::GetConstant(l, "MissionStatus", luaL_checkstring(l, -1)));
	
	lua_pop(l, 2);

	LUA_DEBUG_END(l, -1);
}

/*
 * Method: AddMission
 *
 * Add a mission to the player's mission list
 *
 * > ref = player:AddMission({
 * >     'type'     = type,
 * >     'client'   = client,
 * >     'due'      = due,
 * >     'reward'   = reward,
 * >     'location' = location,
 * >     'status'   = status,
 * > })
 *
 * The fields passed to this method are used to create a new entry in the
 * player's mission info screen. Its important to note that the data provided
 * here are only used for display. They are not used anywhere else in the
 * engine for any reason whatsoever. It is the responsibility of the script to
 * communicate with the player as the mission progresses, decide when the
 * mission has been complete, make payment for completion, etc.
 *
 * Parameters:
 *
 * <AddMission> takes a table as its only parameter. The fields of that table
 * are as follows
 *
 *   type - type of mission. This can be any string
 *
 *   client - the name of the person that offered the mission
 *
 *   due - due date/time, in seconds since 12:00 01-01-3200
 *
 *   reward - reward for mission completion, in dollars
 *
 *   location - a <SystemPath> for the destination space station
 *
 *   status - a <Constants.MissionStatus> string for the current mission
 *            status
 * 
 * Return:
 *
 *   ref - an integer value for referring to the mission in the future
 *
 * Example:
 *
 * > local ref = player:AddMission({
 * >     'type'     = 'Delivery',
 * >     'client'   = 'Jefferson Ford',
 * >     'due'      = Game.time + 3*24*60*60,    -- three days
 * >     'reward'   = 123.45,
 * >     'location' = SystemPath:New(0,0,0,16),  -- Mars High, Sol
 * >     'status'   = 'ACTIVE',
 * > })
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_player_add_mission(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);

	Mission m;
	_table_to_mission(l, m, true);

	int ref = p->missions.Add(m);
	lua_pushinteger(l, ref);

	return 1;
}

/*
 * Method: GetMission
 *
 * Retrieve a mission from the player's mission list
 *
 * > mission = player:GetMission(ref)
 *
 * Parameters:
 *
 *   ref - the mission reference number returned by <AddMission>
 *
 * Return:
 *
 *   mission - a table containing the mission parameters. The fields of the
 *             table are the same as described in <AddMission>.
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
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

/*
 * Method: UpdateMission
 *
 * Update a mission on the player's mission list
 *
 * > player:UpdateMission(ref, mission)
 *
 * The mission data provided to this method is used to overwrite the existing
 * mission data. The intention is that you will use <GetMission> to retrieve
 * the mission table, make the modifications you need, and then call
 * <UpdateMission> to update it.
 *
 * Parameters:
 *
 *   ref - the mission reference number returned by <AddMission>
 *
 *   mission - a table of mission fields. The fields are the same as those
 *             described in <AddMission>.
 *
 * Example:
 *
 * > local mission = player:GetMission(ref)
 * > mission.status = 'FAILED'
 * > player:UpdateMission(ref, mission)
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
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

/*
 * Method: RemoveMission
 *
 * Remove a mission from the player's mission list
 *
 * > player:RemoveMission(ref)
 *
 * Parameters:
 *
 *   ref - the mission reference number returned by <AddMission>
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_player_remove_mission(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	int ref = luaL_checkinteger(l, 2);
	p->missions.Remove(ref);
	return 0;
}

/*
 * Method: GetMoney
 *
 * Get the player's current money
 *
 * > money = player:GetMoney()
 *
 * Return:
 *
 *   money - the player's money, in dollars
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_player_get_money(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	lua_pushnumber(l, p->GetMoney()*0.01);
	return 1;
} 

/*
 * Method: SetMoney
 *
 * Set the player's money
 *
 * > player:SetMoney(money)
 *
 * Parameters:
 *
 *   money - the new amount of money, in dollars
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_player_set_money(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	float m = luaL_checknumber(l, 2);
	p->SetMoney(Sint64(m*100.0));
	return 0;
} 

/*
 * Method: AddMoney
 *
 * Add an amount to the player's money
 *
 * > money = player:AddMoney(change)
 *
 * Parameters:
 *
 *   change - the amount of money to add to the player's money, in dollars
 *
 * Return:
 *
 *   money - the player's new money, in dollars
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   experimental
 */
static int l_player_add_money(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	float a = luaL_checknumber(l, 2);
	Sint64 m = p->GetMoney() + Sint64(a*100.0);
	p->SetMoney(m);
	lua_pushnumber(l, m*0.01);
	return 1;
}

/*
 * Method: AddCrime
 *
 * Add a crime to the player's criminal record
 *
 * > player:AddCrime(crime, fine)
 *
 * Parameters:
 *
 *   crime - a <Constants.PolitCrime> string describing the crime
 *
 *   fine - an amount to add to the player's fine
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */
static int l_player_add_crime(lua_State *l)
{
	LuaPlayer::GetFromLua(1); // check that the method is being called on a Player object
	Sint64 crimeBitset = LuaConstants::GetConstant(l, "PolitCrime", luaL_checkstring(l, 2));
	Sint64 fine = Sint64(luaL_checknumber(l, 3) * 100.0);
	Polit::AddCrime(crimeBitset, fine);
	return 0;
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
 *   experimental
 */

static int l_get_nav_target(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	LuaBody::PushToLua(p->GetNavTarget());
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
 *   target - a <Body> to which to set the navigation target
 * 
 * Availability:
 * 
 *   alpha 14
 *
 * Status:
 *
 *   experimental
 */

static int l_set_nav_target(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
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
 *   experimental
 */

static int l_get_combat_target(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	LuaBody::PushToLua(p->GetCombatTarget());
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
 *   target - a <Body> to which to set the combat target
 * 
 * Availability:
 * 
 *   alpha 14
 *
 * Status:
 *
 *   experimental
 */

static int l_set_combat_target(lua_State *l)
{
	Player *p = LuaPlayer::GetFromLua(1);
	Body *target = LuaBody::GetFromLua(2);
    p->SetCombatTarget(target);
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

		{ "AddCrime",      l_player_add_crime },

		{ "GetNavTarget",    l_get_nav_target    },
		{ "SetNavTarget",    l_set_nav_target    },
		{ "GetCombatTarget", l_get_combat_target },
		{ "SetCombatTarget", l_set_combat_target },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, NULL, NULL);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, promotion_test);
}
