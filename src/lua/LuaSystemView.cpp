// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "EnumStrings.h"
#include "Game.h"
#include "LuaColor.h"
#include "LuaObject.h"
#include "LuaTable.h"
#include "LuaVector.h"
#include "LuaVector2.h"
#include "Pi.h"
#include "Player.h"
#include "SystemView.h"
#include "core/Log.h"
#include "graphics/Graphics.h"

LuaTable projectable_to_lua_row(const Projectable &p, lua_State *l)
{
	LuaTable proj_table(l, 0, 3);
	proj_table.Set("type", int(p.type));
	if (p.type == Projectable::NONE) return proj_table;
	proj_table.Set("base", int(p.base));
	if (p.base == Projectable::SYSTEMBODY)
		proj_table.Set("ref", const_cast<SystemBody *>(p.ref.sbody));
	else
		proj_table.Set("ref", const_cast<Body *>(p.ref.body));
	return proj_table;
}

static int l_systemview_set_color(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	auto color_index = static_cast<SystemMapViewport::ColorIndex>(EnumStrings::GetValue("SystemViewColorIndex", LuaPull<const char *>(l, 2)));
	auto color_value = LuaColor::CheckFromLua(l, 3);
	sv->GetMap()->SetColor(color_index, color_value);
	return 0;
}

static int l_systemview_set_selected_object(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	Projectable::types type = static_cast<Projectable::types>(luaL_checkinteger(l, 2));
	Projectable::bases base = static_cast<Projectable::bases>(luaL_checkinteger(l, 3));
	if (base == Projectable::SYSTEMBODY)
		sv->GetMap()->SetSelectedObject({ type, base, LuaObject<SystemBody>::CheckFromLua(4) });
	else
		sv->GetMap()->SetSelectedObject({ type, base, LuaObject<Body>::CheckFromLua(4) });
	return 0;
}

/*
 * Method: GetProjectedGrouped
 *
 * Get a table of projected point objects from the SystemView class, having
 * previously grouped them.
 *
 * In the process of rendering the current frame in a SystemView class, an
 * array of visible point objects (centers of planets, ships, special points
 * of orbits) is created. This function creates a lua table based on this
 * array, after grouping them according to certain criteria.
 *
 * Resulting table:
 *
 * {
 * 	*** ORBIT ICONS (apoapsis, periapsis) ***
 *
 * 	{	screenCoordinates: vector2d
 * 		mainObject (see struct Projectable in SystemView.h)
 * 		{	type: Projectable::types
 * 			base: Projectable::bases
 * 			ref: SystemBody* or Body*
 * 			}
 * 		objects (if multiple only, main object is duplicated there)
 * 		{
 * 			{type, base, ref}
 * 			{type, base, ref}
 * 			...
 * 			}
 * 		hasPlayer: boolean
 * 		hasCombatTarget: boolean
 * 		hasNavTarget: boolean
 * 	}
 * 	{ screencoordinates, mainobject, ... }
 * 	{ screencoordinates, mainobject, ... }
 * 	...
 *
 * *** LAGRANGE ICONS ***
 * 	{ screenCoordinates, mainObject, ... }
 * 	{ screenCoordinates, mainObject, ... }
 * 	...
 *
 * *** BODY ICONS (stars, planets, ships) ***
 * 	{ screenCoordinates, mainObject, ... }
 * 	{ screenCoordinates, mainObject, ... }
 * 	...
 *
 * }
 *
 *
 * Availability:
 *
 *   2020-03
 *
 * Status:
 *
 *   experimental
 */

static int l_systemview_get_projected_grouped(lua_State *l)
{
	using GroupInfo = Projectable::GroupInfo;

	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	const vector2d gap = LuaPull<vector2d>(l, 2);

	auto &projected = sv->GetMap()->GetProjected();

	const Body *nav_target = Pi::game->GetPlayer()->GetNavTarget();
	const Body *combat_target = Pi::game->GetPlayer()->GetCombatTarget();

	const char *special_object_lua_name[] = { "hasPlayer", "hasCombatTarget", "hasNavTarget" };
	std::vector<Projectable> specials {
		{ Projectable::OBJECT, Projectable::PLAYER, Pi::player },
		{ Projectable::OBJECT, Projectable::SHIP, combat_target }
	};

	if (nav_target && nav_target->GetSystemBody())
		specials.push_back({ Projectable::OBJECT, Projectable::SYSTEMBODY, nav_target->GetSystemBody() });
	else if (nav_target && nav_target->IsType(ObjectType::SHIP))
		specials.push_back({ Projectable::OBJECT, Projectable::SHIP, nav_target });
	else if (nav_target)
		specials.push_back({ Projectable::OBJECT, Projectable::BODY, nav_target });

	//no need to sort, because the bodies are so recorded in good order
	//because they are written recursively starting from the root
	//body of the system, and ships go after the system bodies
	std::vector<GroupInfo> groups = sv->GetMap()->GroupProjectables(vector2f(gap), specials);

	LuaTable result(l, groups.size(), 0);
	int index = 1;

	// groups are stably ordered by projectable type
	// the sooner is displayed, the more in the background
	// so it goes orbit icons -> lagrange icons -> bodies
	for (GroupInfo &group : groups) {
		LuaTable info_table(l, 0, 6);
		const Projectable &mainObject = projected[group.tracks[0]];
		info_table.Set("screenCoordinates", vector3d(group.screenpos));
		info_table.Set("screenSize", mainObject.screensize);
		info_table.Set("mainObject", projectable_to_lua_row(mainObject, l));
		lua_pop(l, 1);
		if (group.tracks.size() > 1) {
			LuaTable objects_table(l, group.tracks.size(), 0);
			int objects_table_index = 1;
			for (int idx : group.tracks) {
				objects_table.Set(objects_table_index++, projectable_to_lua_row(projected[idx], l));
				lua_pop(l, 1);
			}
			info_table.Set("objects", objects_table);
			lua_pop(l, 1);
		}
		for (size_t object_type = 0; object_type < specials.size(); object_type++)
			info_table.Set(special_object_lua_name[object_type], group.hasSpecial(object_type));
		result.Set(index++, info_table);
		lua_pop(l, 1);
	}

	LuaPush(l, result);
	return 1;
}

static int l_systemview_get_system(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	LuaPush(l, sv->GetMap()->GetCurrentSystem());
	return 1;
}

static int l_systemview_get_selected_object(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	Projectable *p = sv->GetMap()->GetSelectedObject();
	LuaPush(l, projectable_to_lua_row(*p, l));
	return 1;
}

static int l_systemview_clear_selected_object(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	sv->GetMap()->ClearSelectedObject();
	return 0;
}

static int l_systemview_view_selected_object(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	sv->GetMap()->ViewSelectedObject();
	return 0;
}

static int l_systemview_reset_viewpoint(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	sv->GetMap()->ResetViewpoint();
	return 0;
}

static int l_systemview_set_visibility(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	sv->GetMap()->SetVisibility(LuaPull<std::string>(l, 2));
	return 0;
}

static int l_systemview_get_orbit_planner_start_time(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	double t = sv->GetOrbitPlannerStartTime();
	if (std::fabs(t) < 1.)
		lua_pushnil(l);
	else
		LuaPush<double>(l, t);
	return 1;
}

static int l_systemview_get_orbit_planner_time(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	double t = sv->GetMap()->GetTime();
	LuaPush<double>(l, t);
	return 1;
}

static int l_systemview_accelerate_time(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	if (lua_isnil(l, 2))
		sv->GetMap()->SetRealTime();
	else {
		double step = LuaPull<double>(l, 2);
		sv->GetMap()->AccelerateTime(step);
	}
	return 0;
}

static int l_systemview_set_rotate_mode(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	bool b = LuaPull<bool>(l, 2);
	sv->GetMap()->SetRotateMode(b);
	return 0;
}

static int l_systemview_set_zoom_mode(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	bool b = LuaPull<bool>(l, 2);
	sv->GetMap()->SetZoomMode(b);
	return 0;
}

static int l_systemview_get_zoom(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	LuaPush(l, sv->GetMap()->GetZoom());
	return 1;
}

static int l_systemview_atlas_view_planet_gap(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	auto radius = LuaPull<float>(l, 2);
	LuaPush(l, sv->GetMap()->AtlasViewPlanetGap(radius));
	return 1;
}

static int l_systemview_atlas_view_pixel_per_unit(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	LuaPush(l, sv->GetMap()->AtlasViewPixelPerUnit());
	return 1;
}

static int l_systemview_get_display_mode(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	LuaPush(l, EnumStrings::GetString("SystemViewMode", int(sv->GetDisplayMode())));
	return 1;
}

static int l_systemview_set_display_mode(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	int mode = EnumStrings::GetValue("SystemViewMode", LuaPull<const char *>(l, 2));
	sv->SetDisplayMode(SystemView::Mode(mode));
	return 1;
}

static int l_systemview_transfer_planner_get(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	std::string key = LuaPull<std::string>(l, 2);

	TransferPlanner *planner = sv->GetTransferPlanner();
	if (key == "prograde") {
		LuaPush<double>(l, planner->GetDv(TransferPlanner::PROGRADE));
	} else if (key == "normal") {
		LuaPush<double>(l, planner->GetDv(TransferPlanner::NORMAL));
	} else if (key == "radial") {
		LuaPush<double>(l, planner->GetDv(TransferPlanner::RADIAL));
	} else if (key == "starttime") {
		LuaPush<double>(l, planner->GetStartTime());
	} else if (key == "factor") {
		LuaPush<double>(l, planner->GetFactor() * 10); // factor is shown as "x 10"
	} else {
		Warning("Unknown transfer planner key %s\n", key.c_str());
		lua_pushnil(l);
	}
	return 1;
}

static int l_systemview_transfer_planner_add(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	std::string key = LuaPull<std::string>(l, 2);
	double delta = LuaPull<double>(l, 3);

	TransferPlanner *planner = sv->GetTransferPlanner();
	if (key == "prograde") {
		planner->AddDv(TransferPlanner::PROGRADE, delta);
	} else if (key == "normal") {
		planner->AddDv(TransferPlanner::NORMAL, delta);
	} else if (key == "radial") {
		planner->AddDv(TransferPlanner::RADIAL, delta);
	} else if (key == "starttime") {
		planner->AddStartTime(delta);
	} else if (key == "factor") {
		if (delta > 0)
			planner->IncreaseFactor();
		else
			planner->DecreaseFactor();
	} else {
		Warning("Unknown transfer planner key %s\n", key.c_str());
	}
	return 0;
}

static int l_systemview_transfer_planner_reset(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	std::string key = LuaPull<std::string>(l, 2);

	TransferPlanner *planner = sv->GetTransferPlanner();
	if (key == "prograde") {
		planner->ResetDv(TransferPlanner::PROGRADE);
	} else if (key == "normal") {
		planner->ResetDv(TransferPlanner::NORMAL);
	} else if (key == "radial") {
		planner->ResetDv(TransferPlanner::RADIAL);
	} else if (key == "starttime") {
		planner->ResetStartTime();
	} else if (key == "factor") {
		planner->ResetFactor();
	} else {
		Warning("Unknown transfer planner key %s\n", key.c_str());
	}

	return 0;
}

template <>
const char *LuaObject<SystemView>::s_type = "SystemView";

template <>
void LuaObject<SystemView>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {

		{ "GetSystem", l_systemview_get_system },
		{ "ClearSelectedObject", l_systemview_clear_selected_object },
		{ "GetProjectedGrouped", l_systemview_get_projected_grouped },
		{ "GetSelectedObject", l_systemview_get_selected_object },
		{ "GetOrbitPlannerStartTime", l_systemview_get_orbit_planner_start_time },
		{ "GetOrbitPlannerTime", l_systemview_get_orbit_planner_time },
		{ "AccelerateTime", l_systemview_accelerate_time },
		{ "SetSelectedObject", l_systemview_set_selected_object },
		{ "ViewSelectedObject", l_systemview_view_selected_object },
		{ "ResetViewpoint", l_systemview_reset_viewpoint },
		{ "SetVisibility", l_systemview_set_visibility },
		{ "SetColor", l_systemview_set_color },
		{ "SetRotateMode", l_systemview_set_rotate_mode },
		{ "SetZoomMode", l_systemview_set_zoom_mode },
		{ "GetDisplayMode", l_systemview_get_display_mode },
		{ "SetDisplayMode", l_systemview_set_display_mode },
		{ "GetZoom", l_systemview_get_zoom },
		{ "AtlasViewPlanetGap", l_systemview_atlas_view_planet_gap },
		{ "AtlasViewPixelPerUnit", l_systemview_atlas_view_pixel_per_unit },

		{ "TransferPlannerAdd", l_systemview_transfer_planner_add },
		{ "TransferPlannerGet", l_systemview_transfer_planner_get },
		{ "TransferPlannerReset", l_systemview_transfer_planner_reset },

		{ NULL, NULL }
	};

	LuaObjectBase::CreateClass(s_type, nullptr, l_methods, 0, 0);
}
