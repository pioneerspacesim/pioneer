// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "Player.h"
#include "Polit.h"
#include "Pi.h"
#include "Game.h"
#include "SectorView.h"
#include "EnumStrings.h"
#include "galaxy/Galaxy.h"

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
	LuaObject<Player>::CheckFromLua(1); // check that the method is being called on a Player object
	Sint64 crimeBitset = LuaConstants::GetConstantFromArg(l, "PolitCrime", 2);
	Sint64 fine = Sint64(luaL_checknumber(l, 3) * 100.0);
	Polit::AddCrime(crimeBitset, fine);
	return 0;
}

// XXX temporary until crime is moved out to Lua properly
static int l_player_get_crime(lua_State *l)
{
	LuaObject<Player>::CheckFromLua(1); // check that the method is being called on a Player object

	Sint64 crimeBitset, fine;
	Polit::GetCrime(&crimeBitset, &fine);

	lua_newtable(l);
	for (Sint64 i = 0; i < 4; i++) { // hardcoding 4 possible Polit::Crime flags
		if (crimeBitset & (Sint64(1)<<i)) {
			lua_pushstring(l, EnumStrings::GetString("PolitCrime", 1<<i));
			lua_rawseti(l, -2, lua_rawlen(l, -2)+1);
		}
	}

	lua_pushnumber(l, double(fine) * 0.01);

	return 2;
}

static int l_player_clear_crime_fine(lua_State *l)
{
	LuaObject<Player>::CheckFromLua(1); // check that the method is being called on a Player object
	Sint64 crimeBitset, fine;
	Polit::GetCrime(&crimeBitset, &fine);
	Polit::AddCrime(0, -fine);
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
	Player *p = LuaObject<Player>::CheckFromLua(1);
	Body *target = LuaObject<Body>::CheckFromLua(2);
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
	Player *p = LuaObject<Player>::CheckFromLua(1);
	Body *target = LuaObject<Body>::CheckFromLua(2);
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
 *   experimental
 */

static int l_get_hyperspace_target(lua_State *l)
{
	Player *player = LuaObject<Player>::CheckFromLua(1);
	SystemPath target;
	if (Pi::game->IsNormalSpace())
		target = Pi::sectorView->GetHyperspaceTarget();
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
 *   experimental
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
			RefCountedPtr<StarSystem> sys = Pi::GetGalaxy()->GetStarSystem(path);
			// Lua should never be able to get an invalid SystemPath
			// (note: this may change if it becomes possible to remove systems during the game)
			assert(path.bodyIndex < sys->GetNumBodies());
			SystemBody *sbody = sys->GetBodyByPath(path);
			if (!sbody->GetSuperType() == SystemBody::SUPERTYPE_STAR)
				return luaL_error(l, "Player:SetHyperspaceTarget() -- second parameter is not a system path or the path of a star");
		}
		Pi::sectorView->SetHyperspaceTarget(path);
		return 0;
	} else
		return luaL_error(l, "Player:SetHyperspaceTarget() cannot be used while in hyperspace");
}

template <> const char *LuaObject<Player>::s_type = "Player";

template <> void LuaObject<Player>::RegisterClass()
{
	static const char *l_parent = "Ship";

	static const luaL_Reg l_methods[] = {
		{ "IsPlayer", l_player_is_player },

		{ "AddCrime",       l_player_add_crime },
		{ "GetCrime",       l_player_get_crime },
		{ "ClearCrimeFine", l_player_clear_crime_fine },

		{ "GetNavTarget",    l_get_nav_target    },
		{ "SetNavTarget",    l_set_nav_target    },
		{ "GetCombatTarget", l_get_combat_target },
		{ "SetCombatTarget", l_set_combat_target },
		{ "GetHyperspaceTarget", l_get_hyperspace_target },
		{ "SetHyperspaceTarget", l_set_hyperspace_target },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<Player>::DynamicCastPromotionTest);
}
