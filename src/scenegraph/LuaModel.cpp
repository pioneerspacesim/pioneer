// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Model.h"
#include "LuaObject.h"

namespace SceneGraph {

class LuaModel {
public:

};

}

using namespace SceneGraph;

template <> const char *LuaObject<SceneGraph::Model>::s_type = "SceneGraph.Model";

template <> void LuaObject<SceneGraph::Model>::RegisterClass()
{
	static const luaL_Reg l_methods[] = {
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, 0, l_methods, 0, 0);
}

