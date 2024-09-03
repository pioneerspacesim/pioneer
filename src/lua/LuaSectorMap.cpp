// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaColor.h"
#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaVector.h"
#include "SectorMap.h"
#include "galaxy/Galaxy.h"
#include "galaxy/GalaxyGenerator.h"
#include "SectorMapContext.h"
#include "LuaVector2.h"
#include "Pi.h"

/*
 * Class: SectorMap
 *
 * A large-scale 3D galaxy map viewer.
 *
 * It is able to scale, rotate the view, move along the specified <SystemPath>,
 * handle clicks on system labels with calling the specified callback
 *
 * > MyMap = SectorMap(onClickSystem)
 *
 * Parameters:
 *
 *   onClickSystem - required parameter, a function that takes 1 argument -
 *                   the <SystemPath> of the clicked system
 *
 */
static int l_sectormap_new(lua_State *l)
{

	luaL_checktype(l, -1, LUA_TFUNCTION);
	LuaRef lua_closure(l, -1);

	class Callbacks : public SectorMapContext::Callbacks {
		LuaRef m_onClickClosure;
	public:
		Callbacks(LuaRef onClickClosure) : m_onClickClosure(onClickClosure) {}
		void OnClickLabel(const SystemPath &clickedLabel) override
		{
			lua_State *l = m_onClickClosure.GetLua();
			m_onClickClosure.PushCopyToStack();
			LuaObject<SystemPath>::PushToLua(clickedLabel);
			pi_lua_protected_call(l, 1, 0);
		}
		SectorMapContext::DisplayMode GetDisplayMode(const SystemPath &system) override
		{
			return DisplayModes::DEFAULT;
		}
	};

	SectorMapContext context;
	context.galaxy = GalaxyGenerator::Create();
	context.input = Pi::input;
	context.config = Pi::config;
	context.renderer = Pi::renderer;
	context.callbacks = std::make_unique<Callbacks>(lua_closure);
	context.pigui = Pi::pigui;

	LuaObject<SectorMap>::CreateInLua(std::move(context));
	return 1;
}

template <>
const char *LuaObject<SectorMap>::s_type = "SectorMap";

template <>
void LuaObject<SectorMap>::RegisterClass()
{
	static LuaMetaType<SectorMap> metaType(s_type);
	metaType.CreateMetaType(Lua::manager->GetLuaState());
	metaType.StartRecording()
		.AddCallCtor(&l_sectormap_new)
		.AddFunction("GetZoomLevel", &SectorMap::GetZoomLevel)
		.AddFunction("SetDrawVerticalLines", &SectorMap::SetDrawVerticalLines)
		.AddFunction("SetFactionVisible", &SectorMap::SetFactionVisible)
		.AddFunction("SetDrawUninhabitedLabels", &SectorMap::SetDrawUninhabitedLabels)
		.AddFunction("GotoSectorPath", &SectorMap::GotoSector)
		.AddFunction("GotoSystemPath", &SectorMap::GotoSystem)
		.AddFunction("SetRotateMode", &SectorMap::SetRotateMode)
		.AddFunction("SetZoomMode", &SectorMap::SetZoomMode)
		.AddFunction("ResetView", &SectorMap::ResetView)
		.AddFunction("IsCenteredOn", &SectorMap::IsCenteredOn)
		.AddFunction("SetLabelParams", &SectorMap::SetLabelParams)
		.AddFunction("SetLabelsVisibility", &SectorMap::SetLabelsVisibility)
		.AddFunction("SetSize", &SectorMap::SetSize)
		.AddFunction("GetGalaxy", [](lua_State *l, SectorMap *sm) {
				LuaPush(l, sm->GetContext().galaxy);
				return 1;
				})
		.AddFunction("GetFactions", [](lua_State *l, SectorMap *sm) {
			const std::set<const Faction *> visible = sm->GetVisibleFactions();
			const std::set<const Faction *> hidden = sm->GetHiddenFactions();
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
		.AddFunction("SearchNearbyStarSystemsByName", [](lua_State *l, SectorMap *sm) -> int {
			std::string pattern = LuaPull<std::string>(l, 2);
			std::vector<SystemPath> matches = sm->GetNearbyStarSystemsByName(pattern);
			int i = 1;
			lua_newtable(l);
			for (const SystemPath &path : matches) {
				lua_pushnumber(l, i++);
				LuaObject<SystemPath>::PushToLua(path);
				lua_settable(l, -3);
			}
			return 1;
		})
		.AddFunction("Draw", [](lua_State *l, SectorMap *obj) {
			obj->Update(Pi::GetFrameTime());
			obj->DrawEmbed();
			return 0;
		})
		.StopRecording();
	LuaObjectBase::CreateClass(&metaType);
}
