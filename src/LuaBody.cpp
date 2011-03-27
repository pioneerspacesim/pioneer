#include "LuaObject.h"
#include "LuaUtils.h"
#include "Body.h"
#include "StarSystem.h"

static int l_body_get_label(lua_State *l)
{
	Body *b = LuaBody::PullFromLua();
	lua_pushstring(l, b->GetLabel().c_str());
	return 1;
} 

static int l_body_get_seed(lua_State *l)
{
	Body *b = LuaBody::PullFromLua();

	const SBody *sbody = b->GetSBody();
	assert(sbody);

	lua_pushinteger(l, sbody->seed);
	return 1;
}

template <> const char *LuaObject<Body>::s_type = "Body";
template <> const char *LuaObject<Body>::s_inherit = NULL;

template <> const luaL_reg LuaObject<Body>::s_methods[] = {
	{ "get_label", l_body_get_label },
	{ "get_seed",  l_body_get_seed  },
	{ 0, 0 }
};

template <> const luaL_reg LuaObject<Body>::s_meta[] = {
	{ 0, 0 }
};
