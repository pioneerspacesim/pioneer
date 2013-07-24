// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Context.h"
#include "LuaObject.h"
#include "LuaConstants.h"

namespace UI {

class LuaContext {
public:

	static inline UI::Widget *_get_implicit_widget(lua_State *l)
	{
		UI::Context *c = LuaObject<UI::Context>::GetFromLua(1);
		assert(c);

		const int top = lua_gettop(l);
		if (top == 1) return 0; // no extra args

		return UI::Lua::GetWidget(c, l, top);
	}

	static inline void _implicit_set_inner_widget(lua_State *l, UI::Single *s)
	{
		UI::Widget *w = _get_implicit_widget(l);
		if (!w) return;
		s->SetInnerWidget(w);
	}

	static inline void _implicit_set_inner_widget(lua_State *l, UI::Scroller *s)
	{
		UI::Widget *w = _get_implicit_widget(l);
		if (!w) return;
		s->SetInnerWidget(w);
	}

	static inline Uint32 _unpack_flags(lua_State *l, int idx, const char *constants) {
		int table = lua_absindex(l, idx);

		if (!lua_istable(l, table))
			return 0;

		LUA_DEBUG_START(l);

		Uint32 flags = 0;

		lua_pushnil(l);
		while (lua_next(l, table)) {
			flags |= static_cast<Uint32>(LuaConstants::GetConstantFromArg(l, constants, -1));
			lua_pop(l, 1);
		}

		LUA_DEBUG_END(l, 0);

		return flags;
	}

	static int l_hbox(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		if (lua_gettop(l) > 1)
			LuaObject<UI::HBox>::PushToLua(c->HBox(luaL_checknumber(l, 2)));
		else
			LuaObject<UI::HBox>::PushToLua(c->HBox());
		return 1;
	}

	static int l_vbox(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		if (lua_gettop(l) > 1)
			LuaObject<UI::VBox>::PushToLua(c->VBox(luaL_checknumber(l, 2)));
		else
			LuaObject<UI::VBox>::PushToLua(c->VBox());
		return 1;
	}

	static int l_grid(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);

		UI::CellSpec rowSpec(1), colSpec(1);

		if (lua_istable(l, 2))
			rowSpec = UI::CellSpec::FromLuaTable(l, 2);
		else
			rowSpec = UI::CellSpec(luaL_checkinteger(l, 2));

		if (lua_istable(l, 3))
			colSpec = UI::CellSpec::FromLuaTable(l, 3);
		else
			colSpec = UI::CellSpec(luaL_checkinteger(l, 3));

		LuaObject<UI::Grid>::PushToLua(c->Grid(rowSpec, colSpec));
		return 1;
	}

	static int l_table(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::Table>::PushToLua(c->Table());
		return 1;
	}

	static int l_background(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		UI::Background *b = c->Background();
		_implicit_set_inner_widget(l, b);
		LuaObject<UI::Background>::PushToLua(b);
		return 1;
	}

	static int l_colorbackground(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		float r = luaL_checknumber(l, 2);
		float g = luaL_checknumber(l, 3);
		float b = luaL_checknumber(l, 4);
		float a = 1.0f;
		if (lua_gettop(l) > 4)
			a = luaL_checknumber(l, 5);
		UI::ColorBackground *cb = c->ColorBackground(Color(r,g,b,a));
		_implicit_set_inner_widget(l, cb);
		LuaObject<UI::ColorBackground>::PushToLua(cb);
		return 1;
	}

	static int l_gradient(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		Color beginColor = Color::FromLuaTable(l, 2);
		Color endColor = Color::FromLuaTable(l, 3);
		UI::Gradient::Direction direction = static_cast<UI::Gradient::Direction>(LuaConstants::GetConstantFromArg(l, "UIGradientDirection", 4));
		UI::Gradient *g = c->Gradient(beginColor, endColor, direction);
		_implicit_set_inner_widget(l, g);
		LuaObject<UI::Gradient>::PushToLua(g);
		return 1;
	}

	static int l_expand(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		UI::Expand::Direction direction = UI::Expand::BOTH;
		if (lua_gettop(l) > 1)
			direction = static_cast<UI::Expand::Direction>(LuaConstants::GetConstantFromArg(l, "UIExpandDirection", 2));
		UI::Expand *e = c->Expand(direction);
		_implicit_set_inner_widget(l, e);
		LuaObject<UI::Expand>::PushToLua(e);
		return 1;
	}

	static int l_margin(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		int margin = luaL_checkinteger(l, 2);
		UI::Margin::Direction dir = UI::Margin::ALL;
		if (lua_gettop(l) > 2)
			dir = static_cast<UI::Margin::Direction>(LuaConstants::GetConstantFromArg(l, "UIMarginDirection", 3));
		UI::Margin *m = c->Margin(margin, dir);
		_implicit_set_inner_widget(l, m);
		LuaObject<UI::Margin>::PushToLua(m);
		return 1;
	}

	static int l_align(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		UI::Align::Direction dir = static_cast<UI::Align::Direction>(LuaConstants::GetConstantFromArg(l, "UIAlignDirection", 2));
		UI::Align *a = c->Align(dir);
		_implicit_set_inner_widget(l, a);
		LuaObject<UI::Align>::PushToLua(a);
		return 1;
	}

	static int l_scroller(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		UI::Scroller *s = c->Scroller();
		_implicit_set_inner_widget(l, s);
		LuaObject<UI::Scroller>::PushToLua(s);
		return 1;
	}

	static int l_icon(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		const std::string iconName(luaL_checkstring(l, 2));
		LuaObject<UI::Icon>::PushToLua(c->Icon(iconName));
		return 1;
	}

	static int l_image(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		const std::string filename(luaL_checkstring(l, 2));
		Uint32 sizeControlFlags = _unpack_flags(l, 3, "UISizeControl");
		LuaObject<UI::Image>::PushToLua(c->Image(filename, sizeControlFlags));
		return 1;
	}

	static int l_label(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::Label>::PushToLua(c->Label(luaL_checkstring(l, 2)));
		return 1;
	}

	static int l_numberlabel(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		UI::NumberLabel::Format format = UI::NumberLabel::FORMAT_NUMBER;
		if (lua_gettop(l) > 1)
			format = static_cast<UI::NumberLabel::Format>(LuaConstants::GetConstantFromArg(l, "UINumberLabelFormat", 2));
		LuaObject<UI::NumberLabel>::PushToLua(c->NumberLabel(format));
		return 1;
	}

	static int l_multilinetext(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::MultiLineText>::PushToLua(c->MultiLineText(luaL_checkstring(l, 2)));
		return 1;
	}

	static int l_button(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		UI::Button *b = c->Button();
		_implicit_set_inner_widget(l, b);
		LuaObject<UI::Button>::PushToLua(b);
		return 1;
	}

	static int l_checkbox(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::CheckBox>::PushToLua(c->CheckBox());
		return 1;
	}

	static int l_smallbutton(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::SmallButton>::PushToLua(c->SmallButton());
		return 1;
	}

	static int l_hslider(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::HSlider>::PushToLua(c->HSlider());
		return 1;
	}

	static int l_vslider(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::VSlider>::PushToLua(c->VSlider());
		return 1;
	}

	static int l_list(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::List>::PushToLua(c->List());
		return 1;
	}

	static int l_dropdown(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::DropDown>::PushToLua(c->DropDown());
		return 1;
	}

	static int l_gauge(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		LuaObject<UI::Gauge>::PushToLua(c->Gauge());
		return 1;
	}

	static int l_textentry(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		std::string text;
		if (lua_gettop(l) > 1)
			text = luaL_checkstring(l, 2);
		LuaObject<UI::TextEntry>::PushToLua(c->TextEntry(text));
		return 1;
	}

	static int l_attr_templates(lua_State *l) {
		UI::Context *c = LuaObject<UI::Context>::CheckFromLua(1);
		c->GetTemplateStore().PushCopyToStack();
		return 1;
	}
};

}

using namespace UI;

template <> const char *LuaObject<UI::Context>::s_type = "UI.Context";

template <> void LuaObject<UI::Context>::RegisterClass()
{
	static const char *l_parent = "UI.Single";

	static const luaL_Reg l_methods[] = {
		{ "HBox",            LuaContext::l_hbox            },
		{ "VBox",            LuaContext::l_vbox            },
		{ "Grid",            LuaContext::l_grid            },
		{ "Table",           LuaContext::l_table           },
		{ "Background",      LuaContext::l_background      },
		{ "ColorBackground", LuaContext::l_colorbackground },
		{ "Gradient",        LuaContext::l_gradient        },
		{ "Expand",          LuaContext::l_expand          },
		{ "Margin",          LuaContext::l_margin          },
		{ "Align",           LuaContext::l_align           },
		{ "Scroller",        LuaContext::l_scroller        },
		{ "Icon",            LuaContext::l_icon            },
		{ "Image",           LuaContext::l_image           },
		{ "Label",           LuaContext::l_label           },
		{ "MultiLineText",   LuaContext::l_multilinetext   },
		{ "NumberLabel",     LuaContext::l_numberlabel     },
		{ "Button",          LuaContext::l_button          },
		{ "CheckBox",        LuaContext::l_checkbox        },
		{ "SmallButton",     LuaContext::l_smallbutton     },
		{ "HSlider",         LuaContext::l_hslider         },
		{ "VSlider",         LuaContext::l_vslider         },
		{ "List",            LuaContext::l_list            },
		{ "DropDown",        LuaContext::l_dropdown        },
		{ "Gauge",           LuaContext::l_gauge           },
		{ "TextEntry",       LuaContext::l_textentry       },
		{ 0, 0 }
	};

	static const luaL_Reg l_attrs[] = {
		{ "templates", LuaContext::l_attr_templates },
		{ 0, 0 }
	};

	LuaObjectBase::CreateClass(s_type, l_parent, l_methods, l_attrs, 0);
	LuaObjectBase::RegisterPromotion(l_parent, s_type, LuaObject<UI::Context>::DynamicCastPromotionTest);
}
