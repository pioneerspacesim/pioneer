// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Game.h"
#include "LuaColor.h"
#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaVector.h"
#include "SectorView.h"
#include "SectorMap.h"

template <>
const char *LuaObject<SectorView>::s_type = "SectorView";

template <>
void LuaObject<SectorView>::RegisterClass()
{
	static LuaMetaType<SectorView> metaType(s_type);
	metaType.CreateMetaType(Lua::manager->GetLuaState());
	metaType.StartRecording()
		.AddFunction("GetCurrentSystemPath", &SectorView::GetCurrent)
		.AddFunction("SetDrawOutRangeLabels", &SectorView::SetDrawOutRangeLabels)
		.AddFunction("SetAutomaticSystemSelection", &SectorView::SetAutomaticSystemSelection)
		.AddFunction("ClearRoute", &SectorView::ClearRoute)
		.AddFunction("GetSelectedSystemPath", &SectorView::GetSelected)
		.AddFunction("GetHyperspaceTargetSystemPath", &SectorView::GetHyperspaceTarget)
		.AddFunction("AddToRoute", &SectorView::AddToRoute)
		.AddFunction("SwitchToPath", &SectorView::SwitchToPath)
		.AddFunction("ResetView", &SectorView::ResetView)
		.AddFunction("SetHyperspaceTarget", &SectorView::SetHyperspaceTarget)
		.AddFunction("ResetHyperspaceTarget", &SectorView::ResetHyperspaceTarget)
		.AddFunction("GetMap", [](lua_State *l, SectorView *sv) {
			LuaObject<SectorMap>::PushToLua(&sv->GetMap());
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
