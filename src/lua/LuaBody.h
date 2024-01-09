// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Body.h"
#include "Lua.h"
#include "LuaPushPull.h"

void pi_lua_generic_pull(lua_State *l, int index, ObjectType &bodyType);
void pi_lua_generic_push(lua_State *l, ObjectType bodyType);
