#include "LuaObject.h"
#include "LuaUtils.h"
#include "Body.h"

static int l_body_get_label(lua_State *l)
{
	Body *b = LuaBody::PullFromLua();
	lua_pushstring(l, b->GetLabel().c_str());
	return 1;
} 

template <> const char *LuaSubObject<Body>::s_type = "Body";
template <> const char *LuaSubObject<Body>::s_inherit = NULL;

template <> const luaL_reg LuaSubObject<Body>::s_methods[] = {
	{ "get_label", l_body_get_label },
	{ 0, 0 }
};

template <> const luaL_reg LuaSubObject<Body>::s_meta[] = {
	{ 0, 0 }
};
