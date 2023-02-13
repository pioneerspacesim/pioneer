// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PI_LUA_JSON_H
#define PI_LUA_JSON_H

#include "JsonFwd.h"
#include "Lua.h"

namespace LuaJson {

	void Register();

	void PushToLua(lua_State *l, const Json &data);

}

#endif
