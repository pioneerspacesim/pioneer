// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Lua.h"
#include "LuaObject.h"
#include "Model.h"
#include "ModelSkin.h"

namespace SceneGraph {
namespace Lua {

void Init()
{
    LuaObject<SceneGraph::Model>::RegisterClass();
    LuaObject<SceneGraph::ModelSkin>::RegisterClass();
}

}
}

