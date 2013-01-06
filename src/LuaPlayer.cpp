// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
	Player *p = LuaPlayer::CheckFromLua(1);
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
	Player *p = LuaPlayer::CheckFromLua(1);
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
	Player *p = LuaPlayer::CheckFromLua(1);
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
	LuaPlayer::CheckFromLua(1); // check that the method is being called on a Player object
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
	Player *p = LuaPlayer::CheckFromLua(1);
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
	Player *p = LuaPlayer::CheckFromLua(1);
	Body *target = LuaBody::CheckFromLua(2);
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
	Player *p = LuaPlayer::CheckFromLua(1);
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
	Player *p = LuaPlayer::CheckFromLua(1);
	Body *target = LuaBody::CheckFromLua(2);
    p->SetCombatTarget(target);
    return 0;
}

template <> const char *LuaObject<Player>::s_type = "Player";

template <> void LuaObject<Player>::RegisterClass()
{
	static const char *l_parent = "Ship";

	static const luaL_Reg l_methods[] = {
		{ "IsPlayer", l_player_is_player },

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
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Player>::DynamicCastPromotionTest);
}
