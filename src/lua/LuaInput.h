// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef LUAINPUT_H
#define LUAINPUT_H

struct lua_State;

namespace InputBindings {
	struct KeyChord;
	struct JoyAxis;
} // namespace InputBindings

namespace LuaInput {
	void Register();
}

void pi_lua_generic_pull(lua_State *l, int index, InputBindings::KeyChord &out);
void pi_lua_generic_pull(lua_State *l, int index, InputBindings::JoyAxis &out);
void pi_lua_generic_push(lua_State *l, InputBindings::KeyChord inChord);
void pi_lua_generic_push(lua_State *l, InputBindings::JoyAxis inAxis);

#endif
