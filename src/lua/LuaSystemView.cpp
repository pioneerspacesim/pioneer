// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
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
#include "graphics/Graphics.h"

LuaTable projectable_to_lua_row(Projectable &p, lua_State *l)
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
	auto color_index = static_cast<SystemView::ColorIndex>(EnumStrings::GetValue("SystemViewColorIndex", LuaPull<const char *>(l, 2)));
	auto color_value = LuaColor::CheckFromLua(l, 3);
	sv->SetColor(color_index, color_value);
	return 0;
}

static int l_systemview_set_selected_object(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	Projectable::types type = static_cast<Projectable::types>(luaL_checkinteger(l, 2));
	Projectable::bases base = static_cast<Projectable::bases>(luaL_checkinteger(l, 3));
	if (base == Projectable::SYSTEMBODY)
		sv->SetSelectedObject(type, base, LuaObject<SystemBody>::CheckFromLua(4));
	else
		sv->SetSelectedObject(type, base, LuaObject<Body>::CheckFromLua(4));
	return 0;
}

bool too_near(const vector3d &a, const vector3d &b, const vector2d &gain)
{
	return std::abs(a.x - b.x) < gain.x && std::abs(a.y - b.y) < gain.y
		// we don’t want to group objects that simply overlap and are located at different distances
		// therefore, depth is also taken into account, we have z_NDC (normalized device coordinates)
		// in order to make a strict translation of delta z_NDC into delta "pixels", one also needs to know
		// the z coordinates in the camera space. I plan to implement this later
		// at the moment I just picked up a number that works well (6.0)
		&& std::abs(a.z - b.z) * Graphics::GetScreenWidth() * 6.0 < gain.x;
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
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	const vector2d gap = LuaPull<vector2d>(l, 2);

	std::vector<Projectable> projected = sv->GetProjected();

	// types of special object
	const int NOT_SPECIAL = -1;
	const int SO_PLAYER = 0;
	const int SO_COMBATTARGET = 1;
	const int SO_NAVTARGET = 2;
	const int NUMBER_OF_SO_TYPES = 3;
	Projectable *special_object[NUMBER_OF_SO_TYPES] = { nullptr, nullptr, nullptr };
	const char *special_object_lua_name[NUMBER_OF_SO_TYPES] = { "hasPlayer", "hasCombatTarget", "hasNavTarget" };

	struct GroupInfo {
		Projectable m_mainObject;
		std::vector<Projectable> m_objects;
		bool m_hasSpecialObject[NUMBER_OF_SO_TYPES] = { false, false, false };

		GroupInfo(Projectable p) :
			m_mainObject(p)
		{
			m_objects.push_back(p);
		}
	};

	// use forward_list to concatenate
	std::vector<GroupInfo> bodyIcons;
	std::vector<GroupInfo> orbitIcons;
	std::vector<GroupInfo> lagrangeIcons;

	const Body *nav_target = Pi::game->GetPlayer()->GetNavTarget();
	const Body *combat_target = Pi::game->GetPlayer()->GetCombatTarget();

	for (Projectable &p : projected) {
		// --- icons---
		if (p.type == Projectable::APOAPSIS || p.type == Projectable::PERIAPSIS)
			// orbit icons - just take all
			orbitIcons.push_back(GroupInfo(p));
		else if (p.type == Projectable::L4 || p.type == Projectable::L5) {
			// lagrange icons - take only those who don't intersect with other lagrange icons
			bool intersect = false;
			for (GroupInfo &group : lagrangeIcons) {
				if (too_near(p.screenpos, group.m_mainObject.screenpos, gap)) {
					intersect = true;
					break;
				}
			}
			if (!intersect) lagrangeIcons.push_back(GroupInfo(p));
		} else {
			// --- real objects ---
			int object_type = NOT_SPECIAL;
			bool inserted = false;
			// check if p is special object, and remember it's type
			if (p.base == Projectable::SYSTEMBODY) {
				if (nav_target && p.ref.sbody == nav_target->GetSystemBody()) object_type = SO_NAVTARGET;
				//system body can't be a combat target and can't be a player
			} else {
				if (nav_target && p.ref.body == nav_target)
					object_type = SO_NAVTARGET;
				else if (combat_target && p.ref.body == combat_target)
					object_type = SO_COMBATTARGET;
				else if (p.base == Projectable::PLAYER)
					object_type = SO_PLAYER;
			}
			for (GroupInfo &group : bodyIcons) {
				if (too_near(p.screenpos, group.m_mainObject.screenpos, gap)) {
					// object inside group boundaries
					// special object is not added
					// special objects must be added to nearest group, this group could be not nearest
					if (object_type == NOT_SPECIAL) {
						group.m_objects.push_back(p);
					} else
						// remember it separately
						special_object[object_type] = &p;
					inserted = true;
					break;
				}
			}
			if (!inserted) {
				// object is not inside a group
				// special object is nearest to itself, so we remember it as a new group
				// create new group
				GroupInfo newgroup(p);
				if (object_type != NOT_SPECIAL) newgroup.m_hasSpecialObject[object_type] = true;
				bodyIcons.push_back(std::move(newgroup));
			}
		}
	} // for (Projectable &p : projected)

	// adding overlapping special object to nearest group
	for (int object_type = 0; object_type < NUMBER_OF_SO_TYPES; object_type++)
		if (special_object[object_type]) {
			std::vector<GroupInfo *> touchedGroups;
			// first we get all groups, touched this object
			for (GroupInfo &group : bodyIcons)
				if (too_near(special_object[object_type]->screenpos, group.m_mainObject.screenpos, gap))
					// object inside group boundaries: remember this group
					touchedGroups.push_back(&group);
			//now select the nearest group (if have)
			if (touchedGroups.size()) {
				GroupInfo *nearest = nullptr;
				double min_length = 1e64;
				for (GroupInfo *&g : touchedGroups) {
					double this_length = (g->m_mainObject.screenpos - special_object[object_type]->screenpos).Length();
					if (this_length < min_length) {
						nearest = g;
						min_length = this_length;
					}
				}
				nearest->m_hasSpecialObject[object_type] = true;
				nearest->m_objects.push_back(*special_object[object_type]);
			} else {
				//don't touching any group, create a new one
				GroupInfo newgroup(*special_object[object_type]);
				newgroup.m_hasSpecialObject[object_type] = true;
				bodyIcons.push_back(std::move(newgroup));
			}
		}

	//no need to sort, because the bodies are so recorded in good order
	//because they are written recursively starting from the root
	//body of the system, and ships go after the system bodies

	LuaTable result(l, orbitIcons.size() + lagrangeIcons.size() + bodyIcons.size(), 0);
	int index = 1;
	//the sooner is displayed, the more in the background
	// so it goes orbitIcons->lagrangeIcons->bodies
	for (auto groups : { orbitIcons, lagrangeIcons, bodyIcons }) {
		for (GroupInfo &group : groups) {
			LuaTable info_table(l, 0, 6);
			info_table.Set("screenCoordinates", group.m_mainObject.screenpos);
			info_table.Set("screenSize", group.m_mainObject.screensize);
			info_table.Set("mainObject", projectable_to_lua_row(group.m_mainObject, l));
			lua_pop(l, 1);
			if (group.m_objects.size() > 1) {
				LuaTable objects_table(l, group.m_objects.size(), 0);
				int objects_table_index = 1;
				for (Projectable &pj : group.m_objects) {
					objects_table.Set(objects_table_index++, projectable_to_lua_row(pj, l));
					lua_pop(l, 1);
				}
				info_table.Set("objects", objects_table);
				lua_pop(l, 1);
			}
			for (int object_type = 0; object_type < NUMBER_OF_SO_TYPES; object_type++)
				info_table.Set(special_object_lua_name[object_type], group.m_hasSpecialObject[object_type]);
			result.Set(index++, info_table);
			lua_pop(l, 1);
		}
	}
	LuaPush(l, result);
	return 1;
}

static int l_systemview_get_system(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	LuaPush(l, sv->GetCurrentSystem());
	return 1;
}

static int l_systemview_get_selected_object(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	Projectable *p = sv->GetSelectedObject();
	LuaPush(l, projectable_to_lua_row(*p, l));
	return 1;
}

static int l_systemview_clear_selected_object(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	sv->ClearSelectedObject();
	return 0;
}

static int l_systemview_view_selected_object(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	sv->ViewSelectedObject();
	return 0;
}

static int l_systemview_reset_viewpoint(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	sv->ResetViewpoint();
	return 0;
}

static int l_systemview_set_visibility(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	sv->SetVisibility(LuaPull<std::string>(l, 2));
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
	double t = sv->GetOrbitPlannerTime();
	LuaPush<double>(l, t);
	return 1;
}

static int l_systemview_accelerate_time(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	if (lua_isnil(l, 2))
		sv->SetRealTime();
	else {
		double step = LuaPull<double>(l, 2);
		sv->AccelerateTime(step);
	}
	return 0;
}

static int l_systemview_set_rotate_mode(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	bool b = LuaPull<bool>(l, 2);
	sv->SetRotateMode(b);
	return 0;
}

static int l_systemview_set_zoom_mode(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	bool b = LuaPull<bool>(l, 2);
	sv->SetZoomMode(b);
	return 0;
}

static int l_systemview_get_zoom(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	LuaPush(l, sv->GetZoom());
	return 1;
}

static int l_systemview_atlas_view_planet_gap(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	auto radius = LuaPull<float>(l, 2);
	LuaPush(l, sv->AtlasViewPlanetGap(radius));
	return 1;
}

static int l_systemview_atlas_view_pixel_per_unit(lua_State *l)
{
	SystemView *sv = LuaObject<SystemView>::CheckFromLua(1);
	LuaPush(l, sv->AtlasViewPixelPerUnit());
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
