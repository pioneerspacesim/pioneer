// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "Player.h"
#include "Pi.h"
#include "Game.h"
#include "SpaceStation.h"
#include "Body.h"
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

/*
 * Method: AutopilotFlyTo
 *
 * Activate the autopilot to fly to the vicinity of a target body.
 *
 * > player.AutopilotFlyTo(body)
 *
 * Parameters:
 *
 *   body - a <Body> to which to fly to.
 *
 * Availability:
 *
 *   alpha XX
 *
 * Status:
 *
 *   experimental
 */
static int l_autopilot_fly_to(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	if(!b) {
		Output("Warning: lua AutopilotFlyTo called with a parameter that is not a Body.\n");
		return 0;
	}
	Pi::player->GetPlayerController()->SetFlightControlState(CONTROL_AUTOPILOT);
	Pi::player->AIFlyTo(b);
	return 0;
}

/*
 * Method: AutopilotOrbitAt
 *
 * Activate the autopilot to orbit a target body at a distance.
 *
 * > player.AutopilotOrbitAt(body, altitude)
 *
 * Parameters:
 *
 *   body - a <Body> which to orbit.
 *   altitude - an altitude number.
 *
 * Availability:
 *
 *   alpha XX
 *
 * Status:
 *
 *   experimental
 */
static int l_autopilot_orbit_at(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	if(!b) {
		Output("Warning: lua AutopilotOrbitAt called with a parameter that is not a Body.\n");
		return 0;
	}
	double alt = luaL_checknumber(l, 2);
	Pi::player->GetPlayerController()->SetFlightControlState(CONTROL_AUTOPILOT);
	Pi::player->AIOrbit(b, alt);
	return 0;
}

/*
 * Method: AutopilotDockWith
 *
 * Activate the autopilot to dock at a target body (must be a station).
 *
 * > player.AutopilotDockWith(body)
 *
 * Parameters:
 *
 *   body - a <Body> which to dock with.
 *
 * Availability:
 *
 *   alpha XX
 *
 * Status:
 *
 *   experimental
 */
static int l_autopilot_dock_with(lua_State *l)
{
	Body *b = LuaObject<Body>::CheckFromLua(1);
	if(!b) {
		Output("Warning: lua AutopilotDockWith called with a parameter that is not a Body.\n");
		return 0;
	}
	if(Pi::player->GetFlightState() != Ship::FLYING)
		return 0;
	SpaceStation *space_station = dynamic_cast<SpaceStation*>(b);
	if(space_station != nullptr) {
		Pi::player->GetPlayerController()->SetFlightControlState(CONTROL_AUTOPILOT);
		Pi::player->AIDock(space_station);
	} else {
		Output("Warning: lua AutopilotDockWith called with a parameter that is not a SpaceStation.\n");
	}
	return 0;
}

template <> const char *LuaObject<Player>::s_type = "Player";

template <> void LuaObject<Player>::RegisterClass()
{
	static const char *l_parent = "Ship";

	static const luaL_Reg l_methods[] = {
		{ "IsPlayer", l_player_is_player },

		{ "AutopilotFlyTo",     l_autopilot_fly_to    },
		{ "AutopilotDockWith",  l_autopilot_dock_with },
		{ "AutopilotOrbitAt",   l_autopilot_orbit_at  },
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
