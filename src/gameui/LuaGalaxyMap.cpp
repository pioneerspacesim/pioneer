// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
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

	static int l_set_centre_sector(lua_State *l) {
		GalaxyMap *map = LuaObject<GalaxyMap>::CheckFromLua(1);
		float x = luaL_checknumber(l, 2);
		float y = luaL_checknumber(l, 3);
		map->SetCentreSector(vector2f(x, y));
		lua_settop(l, 1);
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
		{ "SetCentreSector", &LuaGalaxyMap::l_set_centre_sector },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<GameUI::GalaxyMap>::DynamicCastPromotionTest);
}
