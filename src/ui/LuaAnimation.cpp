// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Animation.h"
#include "LuaObject.h"

namespace UI {

class LuaAnimation {
public:

	static int l_attr_running(lua_State *l) {
		Animation *a = LuaObject<UI::Animation>::CheckFromLua(1);
		lua_pushboolean(l, a->IsRunning());
		return 1;
	}

	static int l_attr_completed(lua_State *l) {
		Animation *a = LuaObject<UI::Animation>::CheckFromLua(1);
		lua_pushboolean(l, a->IsCompleted());
		return 1;
	}

};

}

using namespace UI;

template <> const char *LuaObject<UI::Animation>::s_type = "UI.Animation";

template <> void LuaObject<UI::Animation>::RegisterClass()
{
	static const luaL_Reg l_attrs[] = {
		{ "running",   LuaAnimation::l_attr_running   },
		{ "completed", LuaAnimation::l_attr_completed },
		{ nullptr, nullptr }
	};

	LuaObjectBase::CreateClass(s_type, nullptr, nullptr, l_attrs, nullptr);
}
