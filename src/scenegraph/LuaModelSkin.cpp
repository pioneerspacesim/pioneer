// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelSkin.h"
#include "LuaObject.h"

namespace SceneGraph {

class LuaModelSkin {
public:

	static int l_set_pattern(lua_State *l)
	{
		return 0;
	}

	static int l_set_colors(lua_State *l)
	{
		return 0;
	}

	static int l_set_random_colors(lua_State *l)
	{
		return 0;
	}

	static int l_set_decal(lua_State *l)
	{
		return 0;
	}

	static int l_clear_decal(lua_State *l)
	{
		return 0;
	}

	static int l_clear_decals(lua_State *l)
	{
		return 0;
	}

	static int l_set_label(lua_State *l)
	{
		return 0;
	}

};

}

using namespace SceneGraph;

template <> const char *LuaObject<SceneGraph::ModelSkin>::s_type = "SceneGraph.ModelSkin";

template <> void LuaObject<SceneGraph::ModelSkin>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ "SetPattern",      LuaModelSkin::l_set_pattern  },
		{ "SetColors",       LuaModelSkin::l_set_colors   },
		{ "SetRandomColors", LuaModelSkin::l_set_colors   },
		{ "SetDecal",        LuaModelSkin::l_set_decal    },
		{ "ClearDecal",      LuaModelSkin::l_clear_decal  },
		{ "ClearDecals",     LuaModelSkin::l_clear_decals },
		{ "SetLabel",        LuaModelSkin::l_set_label    },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, 0, 0);
}

