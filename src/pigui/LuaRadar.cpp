// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaPiGui.h"
#include "Radar.h"
#include "lua/Lua.h"
#include "lua/LuaMetaType.h"
#include "lua/LuaPiGuiInternal.h"
#include "lua/LuaVector2.h"

using RadarWidget = PiGui::RadarWidget;

template <>
const char *LuaObject<RadarWidget>::s_type = "PiGui.Modules.RadarWidget";

template <>
void LuaObject<RadarWidget>::RegisterClass()
{
	static LuaMetaType<RadarWidget> s_metaType(s_type);

	s_metaType.CreateMetaType(Lua::manager->GetLuaState());
	s_metaType.StartRecording()
		.AddMember("size", &RadarWidget::GetSize, &RadarWidget::SetSize)
		.AddMember("zoom", &RadarWidget::GetCurrentZoom, &RadarWidget::SetCurrentZoom)
		.AddMember("maxZoom", &RadarWidget::GetMaxZoom, &RadarWidget::SetMaxZoom)
		.AddMember("minZoom", &RadarWidget::GetMinZoom, &RadarWidget::SetMinZoom)
		.AddMember("radius", &RadarWidget::GetRadius)
		.AddMember("center", &RadarWidget::GetCenter)
		.AddFunction("Draw", &RadarWidget::DrawPiGui)
		.StopRecording();

	LuaObjectBase::CreateClass(&s_metaType);

	lua_State *l = Lua::manager->GetLuaState();
	LUA_DEBUG_START(l);

	// Set the metatable to allow calling ModelSpinner() to create a new instance.
	pi_lua_split_table_path(l, s_type); // table, name
	lua_gettable(l, -2);

	lua_newtable(l);
	lua_pushcfunction(l, [](lua_State *l) {
		PushToLua(new RadarWidget());
		return 1;
	});
	lua_setfield(l, -2, "__call");
	lua_setmetatable(l, -2);
	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}
