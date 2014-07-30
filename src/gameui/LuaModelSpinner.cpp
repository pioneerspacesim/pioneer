// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelSpinner.h"
#include "LuaObject.h"
#include "Pi.h"

namespace GameUI {

class LuaModelSpinner {
public:

	static int l_new(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		const std::string name(luaL_checkstring(l, 2));
		SceneGraph::ModelSkin noskin, *skin = &noskin;
		if (lua_gettop(l) > 2)
			skin = LuaObject<SceneGraph::ModelSkin>::CheckFromLua(3);
		unsigned int pattern = 0;
		if (lua_gettop(l) > 3 && !lua_isnoneornil(l, 4))
			pattern = luaL_checkinteger(l, 4) - 1; // Lua counts from 1
		SceneGraph::Model *model = Pi::FindModel(name);
		LuaObject<ModelSpinner>::PushToLua(new ModelSpinner(c, model, *skin, pattern));
		return 1;
	}

	static int l_attr_model(lua_State *l) {
		ModelSpinner *ms = LuaObject<ModelSpinner>::CheckFromLua(1);
		LuaObject<SceneGraph::Model>::PushToLua(ms->GetModel());
		return 1;
	}

};

}

using namespace GameUI;

template <> const char *LuaObject<GameUI::ModelSpinner>::s_type = "UI.Game.ModelSpinner";

template <> void LuaObject<GameUI::ModelSpinner>::RegisterClass()
{
	static const char *l_parent = "UI.Widget";

	static const luaL_Reg l_methods[] = {
		{ "New", LuaModelSpinner::l_new },
        { 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "model", LuaModelSpinner::l_attr_model },
        { 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<GameUI::ModelSpinner>::DynamicCastPromotionTest);
}
