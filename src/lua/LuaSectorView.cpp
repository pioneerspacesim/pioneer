// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Game.h"
#include "LuaColor.h"
#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaVector.h"
#include "SectorView.h"

template <>
const char *LuaObject<SectorView>::s_type = "SectorView";

template <>
void LuaObject<SectorView>::RegisterClass()
{
	static LuaMetaType<SectorView> metaType(s_type);
	metaType.CreateMetaType(Lua::manager->GetLuaState());
	metaType.StartRecording()
		.AddFunction("GetCenterDistance", &SectorView::GetCenterDistance)
		.AddFunction("GetZoomLevel", &SectorView::GetZoomLevel)
		.AddFunction("ZoomIn", &SectorView::ZoomIn)
		.AddFunction("ZoomOut", &SectorView::ZoomOut)
		.AddFunction("GetCurrentSystemPath", &SectorView::GetCurrent)
		.AddFunction("SetDrawVerticalLines", &SectorView::SetDrawVerticalLines)
		.AddFunction("SetDrawOutRangeLabels", &SectorView::SetDrawOutRangeLabels)
		.AddFunction("SetAutomaticSystemSelection", &SectorView::SetAutomaticSystemSelection)
		.AddFunction("SetFactionVisible", &SectorView::SetFactionVisible)
		.AddFunction("ClearRoute", &SectorView::ClearRoute)
		.AddFunction("GetSelectedSystemPath", &SectorView::GetSelected)
		.AddFunction("GetHyperspaceTargetSystemPath", &SectorView::GetHyperspaceTarget)
		.AddFunction("SetDrawUninhabitedLabels", &SectorView::SetDrawUninhabitedLabels)
		.AddFunction("AddToRoute", &SectorView::AddToRoute)
		.AddFunction("SwitchToPath", &SectorView::SwitchToPath)
		.AddFunction("GotoSectorPath", &SectorView::GotoSector)
		.AddFunction("GotoSystemPath", &SectorView::GotoSystem)
		.AddFunction("SetRotateMode", &SectorView::SetRotateMode)
		.AddFunction("SetZoomMode", &SectorView::SetZoomMode)
		.AddFunction("ResetView", &SectorView::ResetView)
		.AddFunction("SetHyperspaceTarget", &SectorView::SetHyperspaceTarget)
		.AddFunction("ResetHyperspaceTarget", &SectorView::ResetHyperspaceTarget)
		.AddFunction("IsCenteredOn", &SectorView::IsCenteredOn)
		.AddFunction("SetLabelParams", &SectorView::SetLabelParams)
		.AddFunction("DrawLabels", &SectorView::DrawLabels)
		.AddFunction("SetLabelsVisibility", &SectorView::SetLabelsVisibility)
		.AddFunction("GetFactions", [](lua_State *l, SectorView *sv) -> int {
			const std::set<const Faction *> visible = sv->GetVisibleFactions();
			const std::set<const Faction *> hidden = sv->GetHiddenFactions();
			lua_newtable(l); // outer table
			int i = 1;
			for (const Faction *f : visible) {
				lua_pushnumber(l, i++);
				lua_newtable(l); // inner table
				LuaObject<Faction>::PushToLua(const_cast<Faction *>(f));
				lua_setfield(l, -2, "faction");
				lua_pushboolean(l, hidden.count(f) == 0);
				lua_setfield(l, -2, "visible"); // inner table
				lua_settable(l, -3);			// outer table
			}
			return 1;
		})
		.AddFunction("GetCenterSector", [](lua_State *l, SectorView *sv) -> int {
			LuaPush<vector3d>(l, vector3d(sv->GetCenterSector()));
			return 1;
		})
		.AddFunction("MoveRouteItemUp", [](lua_State *l, SectorView *sv) -> int {
			int element = LuaPull<int>(l, 2);
			// lua indexes start at 1
			element -= 1;
			bool r = sv->MoveRouteItemUp(element);
			LuaPush<bool>(l, r);
			return 1;
		})
		.AddFunction("MoveRouteItemDown", [](lua_State *l, SectorView *sv) -> int {
			int element = LuaPull<int>(l, 2);
			// lua indexes start at 1
			element -= 1;
			bool r = sv->MoveRouteItemDown(element);
			LuaPush<bool>(l, r);
			return 1;
		})
		.AddFunction("RemoveRouteItem", [](lua_State *l, SectorView *sv) -> int {
			int element = LuaPull<int>(l, 2);
			// lua indexes start at 1
			element -= 1;
			bool r = sv->RemoveRouteItem(element);
			LuaPush<bool>(l, r);
			return 1;
		})
		.AddFunction("UpdateRouteItem", [](lua_State *l, SectorView *sv) -> int {
			int element = LuaPull<int>(l, 2) - 1;
			SystemPath *path = LuaObject<SystemPath>::CheckFromLua(3);
			sv->UpdateRouteItem(element, path);
			return 0;
		})
		.AddFunction("SearchNearbyStarSystemsByName", [](lua_State *l, SectorView *sv) -> int {
			std::string pattern = LuaPull<std::string>(l, 2);
			std::vector<SystemPath> matches = sv->GetNearbyStarSystemsByName(pattern);
			int i = 1;
			lua_newtable(l);
			for (const SystemPath &path : matches) {
				lua_pushnumber(l, i++);
				LuaObject<SystemPath>::PushToLua(path);
				lua_settable(l, -3);
			}
			return 1;
		})
		.AddFunction("GetRouteSize", [](lua_State *l, SectorView *sv) -> int {
			std::vector<SystemPath> route = sv->GetRoute();
			const int size = route.size();
			LuaPush(l, size);
			return 1;
		})
		.AddFunction("AutoRoute", [](lua_State *l, SectorView *sv) {
			SystemPath current_path = sv->GetCurrent();
			SystemPath target_path = sv->GetSelected();
			std::vector<SystemPath> route;
			const std::string result = sv->AutoRoute(current_path, target_path, route);
			if (result == "OKAY") {
				sv->ClearRoute();
				for (auto it = route.begin(); it != route.end(); it++) {
					sv->AddToRoute(*it);
				}
			}
			LuaPush<std::string>(l, result);
			return 1;
		})
		.AddFunction("GetRoute", [](lua_State *l, SectorView *sv) {
			std::vector<SystemPath> route = sv->GetRoute();
			lua_newtable(l);
			int i = 1;
			for (const SystemPath &j : route) {
				lua_pushnumber(l, i++);
				LuaObject<SystemPath>::PushToLua(j);
				lua_settable(l, -3);
			}
			return 1;
		})
		.StopRecording();
	LuaObjectBase::CreateClass(&metaType);
}
