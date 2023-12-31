// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Model.h"
#include "lua/LuaConstants.h"
#include "lua/LuaObject.h"

namespace SceneGraph {

	class LuaModel {
	public:
		static int l_attr_name(lua_State *l)
		{
			SceneGraph::Model *model = LuaObject<SceneGraph::Model>::CheckFromLua(1);
			const std::string &name = model->GetName();
			lua_pushlstring(l, name.c_str(), name.size());
			return 1;
		}

		static int l_set_pattern(lua_State *l)
		{
			SceneGraph::Model *model = LuaObject<SceneGraph::Model>::CheckFromLua(1);
			if (!model->SupportsPatterns())
				return luaL_error(l, "model '%s' does not support patterns", model->GetName().c_str());
			unsigned int pattern = luaL_checkinteger(l, 2);
			pattern--; // Lua counts from 1
			if (pattern >= model->GetNumPatterns())
				return luaL_error(l, "invalid pattern number '%d'; model '%s' has patterns 1-%d", pattern + 1, model->GetName().c_str(), model->GetNumPatterns());
			model->SetPattern(pattern);
			return 0;
		}

		static int l_attr_pattern(lua_State *l)
		{
			SceneGraph::Model *model = LuaObject<SceneGraph::Model>::CheckFromLua(1);
			if (!model->SupportsPatterns())
				return 0;
			lua_pushinteger(l, model->GetPattern() + 1); // Lua counts from 1
			return 1;
		}

		static int l_attr_num_patterns(lua_State *l)
		{
			SceneGraph::Model *model = LuaObject<SceneGraph::Model>::CheckFromLua(1);
			if (!model->SupportsPatterns())
				lua_pushinteger(l, 0);
			else
				lua_pushinteger(l, model->GetNumPatterns());
			return 1;
		}

		static inline Uint32 _unpack_flags(lua_State *l, int idx, const char *constants)
		{
			int table = lua_absindex(l, idx);

			if (!lua_istable(l, table))
				return 0;

			LUA_DEBUG_START(l);

			Uint32 flags = 0;

			lua_pushnil(l);
			while (lua_next(l, table)) {
				flags |= static_cast<Uint32>(LuaConstants::GetConstantFromArg(l, constants, -1));
				lua_pop(l, 1);
			}

			LUA_DEBUG_END(l, 0);

			return flags;
		}

		static int l_set_debug_flags(lua_State *l)
		{
			SceneGraph::Model *model = LuaObject<SceneGraph::Model>::CheckFromLua(1);
			Uint32 debugFlags = _unpack_flags(l, 2, "ModelDebugFlags");
			model->SetDebugFlags(debugFlags);
			return 0;
		}
	};

} // namespace SceneGraph

using namespace SceneGraph;

template <>
const char *LuaObject<SceneGraph::Model>::s_type = "SceneGraph.Model";

template <>
void LuaObject<SceneGraph::Model>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "SetPattern", LuaModel::l_set_pattern },
		{ "SetDebugFlags", LuaModel::l_set_debug_flags },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "name", LuaModel::l_attr_name },
		{ "pattern", LuaModel::l_attr_pattern },
		{ "numPatterns", LuaModel::l_attr_num_patterns },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, l_attrs, 0);
}
