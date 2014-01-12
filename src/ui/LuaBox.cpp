// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Box.h"
#include "Lua.h"
#include "LuaConstants.h"

namespace UI {

class LuaBox {
public:

	static int l_pack_start(lua_State *l) {
		UI::Box *b = LuaObject<UI::Box>::CheckFromLua(1);
		UI::Context *c = b->GetContext();

		if (lua_istable(l, 2)) {
			UI::Widget *w = UI::Lua::GetWidget(c, l, 2);
			if (w)
				b->PackStart(w);
			else
				for (size_t i = lua_rawlen(l, 2); i > 0; i--) {
					lua_rawgeti(l, 2, i);
					b->PackStart(UI::Lua::CheckWidget(c, l, -1));
					lua_pop(l, 1);
				}
		}
		else
			b->PackStart(UI::Lua::CheckWidget(c, l, 2));

		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_pack_end(lua_State *l) {
		UI::Box *b = LuaObject<UI::Box>::CheckFromLua(1);
		UI::Context *c = b->GetContext();

		if (lua_istable(l, 2)) {
			UI::Widget *w = UI::Lua::GetWidget(c, l, 2);
			if (w)
				b->PackEnd(w);
			else
				for (size_t i = 0; i < lua_rawlen(l, 2); i++) {
					lua_rawgeti(l, 2, i+1);
					b->PackEnd(UI::Lua::CheckWidget(c, l, -1));
					lua_pop(l, 1);
				}
		}
		else
			b->PackEnd(UI::Lua::CheckWidget(c, l, 2));

		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_remove(lua_State *l) {
		UI::Box *b = LuaObject<UI::Box>::CheckFromLua(1);
		UI::Context *c = b->GetContext();
		UI::Widget *w = UI::Lua::CheckWidget(c, l, 2);
		b->Remove(w);
		return 0;
	}

	static int l_clear(lua_State *l) {
		UI::Box *b = LuaObject<UI::Box>::CheckFromLua(1);
		b->Clear();
		return 0;
	}

};

class LuaHBox;
class LuaVBox;

}

using namespace UI;

template <> const char *LuaObject<UI::Box>::s_type = "UI.Box";

template <> void LuaObject<UI::Box>::RegisterClass()
{
	static const char *l_parent = "UI.Container";

	static const luaL_Reg l_methods[] = {
		{ "PackStart", LuaBox::l_pack_start },
		{ "PackEnd",   LuaBox::l_pack_end   },
		{ "Remove",    LuaBox::l_remove     },
		{ "Clear",     LuaBox::l_clear      },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Box>::DynamicCastPromotionTest);
}

template <> const char *LuaObject<UI::HBox>::s_type = "UI.HBox";

template <> void LuaObject<UI::HBox>::RegisterClass()
{
	static const char *l_parent = "UI.Box";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::HBox>::DynamicCastPromotionTest);
}

template <> const char *LuaObject<UI::VBox>::s_type = "UI.VBox";

template <> void LuaObject<UI::VBox>::RegisterClass()
{
	static const char *l_parent = "UI.Box";

	LuaObjectBase::CreateClass(s_type, l_parent, 0, 0, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::VBox>::DynamicCastPromotionTest);
}
