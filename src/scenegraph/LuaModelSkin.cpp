// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelSkin.h"
#include "Serializer.h"
#include "lua/LuaColor.h"
#include "lua/LuaObject.h"
#include "lua/LuaTable.h"

namespace SceneGraph {

	class LuaModelSkin {
	public:
		static int l_new(lua_State *l)
		{
			ModelSkin skin;
			LuaObject<ModelSkin>::PushToLua(skin);
			return 1;
		}

		static int l_get_colors(lua_State *l)
		{
			ModelSkin *skin = LuaObject<ModelSkin>::CheckFromLua(1);
			std::vector<Color> v = skin->GetColors();
			LuaTable t = LuaTable(l);
			t.LoadVector(v.begin(), v.end());
			LuaPush(l, t);
			return 1;
		}

		static int l_set_colors(lua_State *l)
		{
			ModelSkin *skin = LuaObject<ModelSkin>::CheckFromLua(1);

			luaL_checktype(l, 2, LUA_TTABLE);

			lua_getfield(l, 2, "primary");
			if (!lua_isnil(l, -1))
				skin->SetPrimaryColor(LuaPull<Color>(l, -1));

			lua_getfield(l, 2, "secondary");
			if (!lua_isnil(l, -1))
				skin->SetSecondaryColor(LuaPull<Color>(l, -1));

			lua_getfield(l, 2, "trim");
			if (!lua_isnil(l, -1))
				skin->SetTrimColor(LuaPull<Color>(l, -1));

			lua_pushvalue(l, 1);
			return 1;
		}

		static int l_set_random_colors(lua_State *l)
		{
			ModelSkin *skin = LuaObject<ModelSkin>::CheckFromLua(1);
			Random *rand = LuaObject<Random>::CheckFromLua(2);
			skin->SetRandomColors(*rand);
			lua_pushvalue(l, 1);
			return 1;
		}

		static int l_set_decal(lua_State *l)
		{
			ModelSkin *skin = LuaObject<ModelSkin>::CheckFromLua(1);
			const char *name = luaL_checkstring(l, 2);
			unsigned int index = 0;
			if (lua_gettop(l) > 2)
				index = luaL_checkinteger(l, 3);
			skin->SetDecal(name, index);
			lua_pushvalue(l, 1);
			return 1;
		}

		static int l_clear_decal(lua_State *l)
		{
			ModelSkin *skin = LuaObject<ModelSkin>::CheckFromLua(1);
			unsigned int index = 0;
			if (lua_gettop(l) > 1)
				index = luaL_checkinteger(l, 3);
			skin->ClearDecal(index);
			lua_pushvalue(l, 1);
			return 1;
		}

		static int l_clear_decals(lua_State *l)
		{
			ModelSkin *skin = LuaObject<ModelSkin>::CheckFromLua(1);
			skin->ClearDecals();
			lua_pushvalue(l, 1);
			return 1;
		}

		static int l_set_label(lua_State *l)
		{
			ModelSkin *skin = LuaObject<ModelSkin>::CheckFromLua(1);
			const char *label = luaL_checkstring(l, 2);
			skin->SetLabel(label);
			lua_pushvalue(l, 1);
			return 1;
		}
	};

} // namespace SceneGraph

static bool _modelskin_to_json(lua_State *l, Json &out)
{
	auto *skin = LuaObject<SceneGraph::ModelSkin>::GetFromLua(-1);
	if (!skin) return false;

	skin->SaveToJson(out);
	return true;
}

static bool _modelskin_from_json(lua_State *l, const Json &obj)
{
	SceneGraph::ModelSkin skin;
	skin.LoadFromJson(obj);
	LuaObject<SceneGraph::ModelSkin>::PushToLua(skin);
	return true;
}

using namespace SceneGraph;

template <>
const char *LuaObject<SceneGraph::ModelSkin>::s_type = "SceneGraph.ModelSkin";

template <>
void LuaObject<SceneGraph::ModelSkin>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "New", LuaModelSkin::l_new },
		{ "GetColors", LuaModelSkin::l_get_colors },
		{ "SetColors", LuaModelSkin::l_set_colors },
		{ "SetRandomColors", LuaModelSkin::l_set_random_colors },
		{ "SetDecal", LuaModelSkin::l_set_decal },
		{ "ClearDecal", LuaModelSkin::l_clear_decal },
		{ "ClearDecals", LuaModelSkin::l_clear_decals },
		{ "SetLabel", LuaModelSkin::l_set_label },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, 0, 0);
	LuaObjectBase::RegisterSerializer(s_type, SerializerPair(_modelskin_to_json, _modelskin_from_json));
}
