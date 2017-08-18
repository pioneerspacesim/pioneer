// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaObject.h"
#include "LuaUtils.h"
#include "LuaConstants.h"
#include "HyperspaceCloud.h"
#include "Pi.h"
#include "Game.h"
#include "Ship.h"
#include "SectorView.h"
#include "EnumStrings.h"
#include "galaxy/Galaxy.h"
/*
 * Class: HyperspaceCloud
 *
 * Class representing a hyperspace cloud. Inherits from <Body>
 */


/* Method: IsArrival
 *
 * Return true if this is an arrival cloud.
 *
 * Returns:
 *
 *    true if this is an arrival cloud.
 *
 */
static int l_hyperspace_cloud_is_arrival(lua_State *l)
{
	HyperspaceCloud *cloud = LuaObject<HyperspaceCloud>::CheckFromLua(1);
	LuaPush(l, cloud->IsArrival());
	return 1;
}


/* Method: GetShip
 *
 * Return the <Ship> that created this cloud, or nil.
 *
 * Returns:
 *
 *    the <Ship> that created this cloud, or nil.
 *
 */
static int l_hyperspace_cloud_get_ship(lua_State *l)
{
	HyperspaceCloud *cloud = LuaObject<HyperspaceCloud>::CheckFromLua(1);
	Ship *ship = cloud->GetShip();
	if(ship == nullptr)
		lua_pushnil(l);
	else
		LuaPush(l, ship);
	return 1;
}


/* Method: GetDueDate
 *
 * Return the date when a ship has entered / will exit this cloud.
 *
 * Returns:
 *
 *    the date when a ship has entered / will exit this cloud
 *
 */
static int l_hyperspace_cloud_get_due_date(lua_State *l)
{
		HyperspaceCloud *cloud = LuaObject<HyperspaceCloud>::CheckFromLua(1);
		LuaPush(l, cloud->GetDueDate());
		return 1;
}

template <> const char *LuaObject<HyperspaceCloud>::s_type = "HyperspaceCloud";

template <> void LuaObject<HyperspaceCloud>::RegisterClass()
{
	static const char *l_parent = "Body";

	static const luaL_Reg l_methods[] = {
		{ "IsArrival",  l_hyperspace_cloud_is_arrival },
		{ "GetShip",    l_hyperspace_cloud_get_ship },
		{ "GetDueDate", l_hyperspace_cloud_get_due_date },
			
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<HyperspaceCloud>::DynamicCastPromotionTest);
}
