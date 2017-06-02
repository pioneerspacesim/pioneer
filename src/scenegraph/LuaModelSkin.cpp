// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelSkin.h"
#include "LuaObject.h"

namespace SceneGraph {

class LuaModelSkin {
public:

	static int l_new(lua_State *l)
	{
		ModelSkin skin;
		LuaObject<ModelSkin>::PushToLua(skin);
		return 1;
	}

	static int l_set_colors(lua_State *l)
	{
		ModelSkin *skin = LuaObject<ModelSkin>::CheckFromLua(1);

		luaL_checktype(l, 2, LUA_TTABLE);

		lua_getfield(l, 2, "primary");
		if (lua_istable(l, -1))
			skin->SetPrimaryColor(Color(Color4f::FromLuaTable(l, -1)));

		lua_getfield(l, 2, "secondary");
		if (lua_istable(l, -1))
			skin->SetSecondaryColor(Color(Color4f::FromLuaTable(l, -1)));

		lua_getfield(l, 2, "trim");
		if (lua_istable(l, -1))
			skin->SetTrimColor(Color(Color4f::FromLuaTable(l, -1)));

		lua_pop(l, 3);

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

}

static std::string _modelskin_serializer(LuaWrappable *o)
{
	static char buf[256];

	SceneGraph::ModelSkin *skin = static_cast<SceneGraph::ModelSkin*>(o);

	Serializer::Writer wr;
	skin->Save(wr);
	const std::string &ser = wr.GetData();
	snprintf(buf, sizeof(buf), SIZET_FMT "\n", ser.size());

	return std::string(buf) + ser;
}

static bool _modelskin_deserializer(const char *pos, const char **next)
{
	const char *end;

	Uint32 serlen = strtoul(pos, const_cast<char**>(&end), 0);
	if (pos == end) return false;
	pos = end+1; // skip newline

	std::string buf(pos, serlen);
	const char *bufp = buf.c_str();
	Serializer::Reader rd(ByteRange(bufp, bufp + buf.size()));
	SceneGraph::ModelSkin skin;
	skin.Load(rd);

	LuaObject<SceneGraph::ModelSkin>::PushToLua(skin);

	*next = pos + serlen;

	return true;
}

using namespace SceneGraph;

template <> const char *LuaObject<SceneGraph::ModelSkin>::s_type = "SceneGraph.ModelSkin";

template <> void LuaObject<SceneGraph::ModelSkin>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "New",             LuaModelSkin::l_new               },
		{ "SetColors",       LuaModelSkin::l_set_colors        },
		{ "SetRandomColors", LuaModelSkin::l_set_random_colors },
		{ "SetDecal",        LuaModelSkin::l_set_decal         },
		{ "ClearDecal",      LuaModelSkin::l_clear_decal       },
		{ "ClearDecals",     LuaModelSkin::l_clear_decals      },
		{ "SetLabel",        LuaModelSkin::l_set_label         },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, 0, 0);
	LuaObjectBase::RegisterSerializer(s_type, SerializerPair(_modelskin_serializer, _modelskin_deserializer));
}

