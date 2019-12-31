// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Lua.h"
#include "OverlayStack.h"

namespace UI {

	class LuaOverlayStack {
	public:
		static int l_add_layer(lua_State *l)
		{
			OverlayStack *s = LuaObject<UI::OverlayStack>::CheckFromLua(1);
			Context *c = s->GetContext();
			Widget *w = UI::Lua::CheckWidget(c, l, 2);
			s->AddLayer(w);
			lua_pushvalue(l, 1);
			return 1;
		}

		static int l_clear(lua_State *l)
		{
			OverlayStack *s = LuaObject<UI::OverlayStack>::CheckFromLua(1);
			s->Clear();
			return 0;
		}
	};

} // namespace UI

using namespace UI;

template <>
const char *LuaObject<UI::OverlayStack>::s_type = "UI.OverlayStack";

template <>
void LuaObject<UI::OverlayStack>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {
		{ "AddLayer", LuaOverlayStack::l_add_layer },
		{ "Clear", LuaOverlayStack::l_clear },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::OverlayStack>::DynamicCastPromotionTest);
}
