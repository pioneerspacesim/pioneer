#include "WidgetSet.h"
#include "Context.h"
#include "LuaObject.h"

namespace UI {

WidgetSet WidgetSet::FromLuaTable(lua_State *l, int idx)
{
	int table = idx > 0 ? idx : lua_gettop(l) - idx;
	assert(lua_istable(l, table));

	Widget *widgets[8];
	for (size_t i = 0; i < 8 && i < lua_rawlen(l, table); i++) {
		lua_rawgeti(l, table, i+1);
		widgets[i] = LuaObject<UI::Widget>::CheckFromLua(-1);
		lua_pop(l, 1);
	}

	return WidgetSet(widgets, lua_rawlen(l, table));
}

}
