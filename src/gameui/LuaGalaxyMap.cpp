// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GalaxyMap.h"
#include "LuaObject.h"
#include "LuaConstants.h"
#include "ui/Lua.h"
#include "ui/LuaSignal.h"
#include "vector2.h"

namespace GameUI {

class LuaGalaxyMap {
public:

	static int l_new(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<GalaxyMap>::PushToLua(new GalaxyMap(c));
		return 1;
	}

	static int l_set_zoom(lua_State *l) {
		GalaxyMap *map = LuaObject<GalaxyMap>::CheckFromLua(1);
		map->SetZoom(luaL_checknumber(l, 2));
		lua_settop(l, 1);
		return 1;
	}

	static int l_clear_labels(lua_State *l) {
		GalaxyMap *map = LuaObject<GalaxyMap>::CheckFromLua(1);
		map->ClearLabels();
		return 0;
	}

	static int l_add_area_label(lua_State *l) {
		GalaxyMap *map = LuaObject<GalaxyMap>::CheckFromLua(1);
		float x = luaL_checknumber(l, 2);
		float y = luaL_checknumber(l, 3);
		std::string text;
		pi_lua_generic_pull(l, 4, text);
		map->AddAreaLabel(vector2f(x, y), text);
		lua_settop(l, 1);
		return 1;
	}

	static int l_add_point_label(lua_State *l) {
		GalaxyMap *map = LuaObject<GalaxyMap>::CheckFromLua(1);
		float x = luaL_checknumber(l, 2);
		float y = luaL_checknumber(l, 3);
		std::string text;
		pi_lua_generic_pull(l, 4, text);
		map->AddPointLabel(vector2f(x, y), text);
		lua_settop(l, 1);
		return 1;
	}

	static int l_set_centre_sector(lua_State *l) {
		GalaxyMap *map = LuaObject<GalaxyMap>::CheckFromLua(1);
		float x = luaL_checknumber(l, 2);
		float y = luaL_checknumber(l, 3);
		map->SetCentreSector(vector2f(x, y));
		lua_settop(l, 1);
		return 1;
	}

	static int l_attr_map_scale(lua_State *l) {
		GalaxyMap *map = LuaObject<GalaxyMap>::CheckFromLua(1);
		lua_pushnumber(l, map->GetDisplayScale());
		return 1;
	}

	static int l_attr_on_map_scale_changed(lua_State *l) {
		GalaxyMap *map = LuaObject<GalaxyMap>::CheckFromLua(1);
		UI::LuaSignal<float>().Wrap(l, map->onDisplayScaleChanged);
		return 1;
	}

};

}

using namespace GameUI;

template <> const char *LuaObject<GameUI::GalaxyMap>::s_type = "UI.Game.GalaxyMap";

template <> void LuaObject<GameUI::GalaxyMap>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {
		{ "New",             &LuaGalaxyMap::l_new },
		{ "SetZoom",         &LuaGalaxyMap::l_set_zoom },
		{ "ClearLabels",     &LuaGalaxyMap::l_clear_labels },
		{ "AddAreaLabel",    &LuaGalaxyMap::l_add_area_label },
		{ "AddPointLabel",   &LuaGalaxyMap::l_add_point_label },
		{ "SetCentreSector", &LuaGalaxyMap::l_set_centre_sector },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "mapScale",          &LuaGalaxyMap::l_attr_map_scale },
		{ "onMapScaleChanged", &LuaGalaxyMap::l_attr_on_map_scale_changed },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<GameUI::GalaxyMap>::DynamicCastPromotionTest);
}
