#include "Box.h"
#include "LuaObject.h"
#include "LuaConstants.h"

namespace UI {

class LuaBox {
public:

	/*
	Box *PackStart(Widget *child, Uint32 flags = 0);
	Box *PackStart(const WidgetSet &set, Uint32 flags = 0);
	Box *PackEnd(Widget *child, Uint32 flags = 0);
	Box *PackEnd(const WidgetSet &set, Uint32 flags = 0);

	void Remove(Widget *child);
	void Clear();
	*/

	static inline void _unpack_pack_args(lua_State *l, WidgetSet &widgetSet, Uint32 &flags) {
		if (lua_istable(l, 2))
			widgetSet = UI::WidgetSet::FromLuaTable(l, 2);
		else
			widgetSet = UI::WidgetSet(LuaObject<UI::Widget>::CheckFromLua(2));

		flags = 0;

		if (lua_gettop(l) > 2) {
			luaL_checktype(l, 3, LUA_TTABLE);

			lua_pushnil(l);
			while (lua_next(l, 3)) {
				flags |= static_cast<Uint32>(LuaConstants::GetConstantFromArg(l, "UIBoxFlags", -1));
				lua_pop(l, 1);
			}
			lua_pop(l, 1);
		}
	}

	static int l_pack_start(lua_State *l) {
		UI::Box *b = LuaObject<UI::Box>::CheckFromLua(1);

		WidgetSet widgetSet;
		Uint32 flags;
		_unpack_pack_args(l, widgetSet, flags);

		b->PackStart(widgetSet, flags);

		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_pack_end(lua_State *l) {
		UI::Box *b = LuaObject<UI::Box>::CheckFromLua(1);

		WidgetSet widgetSet;
		Uint32 flags;
		_unpack_pack_args(l, widgetSet, flags);

		b->PackEnd(widgetSet, flags);

		lua_pushvalue(l, 1);
		return 1;
	}

	static int l_remove(lua_State *l) {
		UI::Box *b = LuaObject<UI::Box>::CheckFromLua(1);
		UI::Widget *w = LuaObject<UI::Widget>::CheckFromLua(2);
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
