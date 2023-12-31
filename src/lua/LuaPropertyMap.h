// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "LuaObject.h"

class Property;

void pi_lua_generic_pull(lua_State *l, int index, Property &out);
void pi_lua_generic_push(lua_State *l, const Property &value);
