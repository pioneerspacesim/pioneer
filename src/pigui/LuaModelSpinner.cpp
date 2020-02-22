
#include "Pi.h"
#include "lua/LuaObject.h"
#include "lua/LuaPushPull.h"
#include "lua/LuaVector.h"
#include "lua/LuaVector2.h"
#include "pigui/ModelSpinner.h"

namespace PiGUI {
	namespace LuaPiguiModelSpinner {
		static int l_model_new(lua_State *l)
		{
			LuaObject<ModelSpinner>::PushToLua(new ModelSpinner());
			return 1;
		}

		static int l_model_set_model(lua_State *l)
		{
			auto *obj = LuaObject<ModelSpinner>::CheckFromLua(1);
			const std::string name(luaL_checkstring(l, 2));
			SceneGraph::ModelSkin *skin = LuaObject<SceneGraph::ModelSkin>::CheckFromLua(3);
			unsigned int pattern = 0;
			if (lua_gettop(l) > 3 && !lua_isnoneornil(l, 4))
				pattern = luaL_checkinteger(l, 4) - 1; // Lua counts from 1
			SceneGraph::Model *model = Pi::FindModel(name);
			obj->SetModel(model, *skin, pattern);

			return 0;
		}

		static int l_model_set_size(lua_State *l)
		{
			auto *obj = LuaObject<ModelSpinner>::CheckFromLua(1);
			vector2d &size = *LuaVector2::CheckFromLua(l, 2);
			obj->SetSize(size);
			return 0;
		}

		static int l_model_attr_size(lua_State *l)
		{
			auto *obj = LuaObject<ModelSpinner>::CheckFromLua(1);
			LuaPush<vector2d>(l, obj->GetSize());
			return 1;
		}

		static int l_model_space_to_screen_space(lua_State *l)
		{
			auto *obj = LuaObject<ModelSpinner>::CheckFromLua(1);
			vector3d modelSpaceVec = LuaPull<vector3d>(l, 2);
			LuaPush<vector2d>(l, obj->ModelSpaceToScreenSpace(modelSpaceVec));
			return 1;
		}

		static int l_model_draw(lua_State *l)
		{
			auto *obj = LuaObject<ModelSpinner>::CheckFromLua(1);
			obj->Render();
			obj->DrawPiGui();

			return 0;
		}
	}; // namespace LuaPiguiModelSpinner
} // namespace PiGUI

using namespace PiGUI::LuaPiguiModelSpinner;

template <>
const char *LuaObject<PiGUI::ModelSpinner>::s_type = "PiGui.Modules.ModelSpinner";

template <>
void LuaObject<PiGUI::ModelSpinner>::RegisterClass()
{

	const luaL_Reg l_meta[] = {
		{ "__call", l_model_new },
		{ NULL, NULL }
	};

	const luaL_Reg l_attrs[] = {
		{ "size", l_model_attr_size },
		{ NULL, NULL }
	};

	const luaL_Reg l_methods[] = {
		{ "new", l_model_new },
		{ "draw", l_model_draw },
		{ "setModel", l_model_set_model },
		{ "setSize", l_model_set_size },
		{ "modelSpaceToScreenSpace", l_model_space_to_screen_space },
		{ NULL, NULL }
	};

	LuaObjectBase::CreateClass(s_type, NULL, l_methods, l_attrs, NULL);

	lua_State *l = Lua::manager->GetLuaState();
	LUA_DEBUG_START(l);

	// Set the metatable to allow calling ModelSpinner() to create a new instance.
	pi_lua_split_table_path(l, s_type); // table, name
	lua_gettable(l, -2);

	lua_newtable(l);
	luaL_setfuncs(l, l_meta, 0);
	lua_setmetatable(l, -2);
	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}
