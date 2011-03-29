#include "LuaSerializer.h"
#include "LuaObject.h"

void LuaSerializer::RegisterSerializer()
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	LUA_DEBUG_START(l)

	LuaObject<LuaSerializer>::PushToLua(this);
	lua_setfield(l, LUA_GLOBALSINDEX, "Serializer");

	LUA_DEBUG_END(l, 0)
}

void LuaSerializer::Serialize(Serializer::Writer &wr)
{
    assert(0);
}

void LuaSerializer::Unserialize(Serializer::Reader &rd)
{
    assert(0);
}

int LuaSerializer::l_connect(lua_State *l)
{
	LUA_DEBUG_START(l)

	if (!lua_isfunction(l, 2))
		luaL_typerror(l, 2, lua_typename(l, LUA_TFUNCTION));
	if (!lua_isfunction(l, 3))
		luaL_typerror(l, 3, lua_typename(l, LUA_TFUNCTION));
	
	lua_Debug ar;
    for (int i = 0; lua_getstack(l, i, &ar) == 1; i++);
	lua_getinfo(l, "S", &ar);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializer");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializer");
		lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializer");
	}

	lua_getfield(l, -1, ar.short_src);
	if(!(lua_isnil(l, -1)))
		luaL_error(l, "Serializer functions for '%s' are already registered\n", ar.short_src);
	lua_pop(l, 1);

	lua_newtable(l);

	lua_pushinteger(l, 1);
	lua_pushvalue(l, 2);
	lua_rawset(l, -3);
	lua_pushinteger(l, 2);
	lua_pushvalue(l, 3);
	lua_rawset(l, -3);

	lua_pushstring(l, ar.short_src);
	lua_pushvalue(l, -2);
	lua_rawset(l, -4);

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0)

	return 0;
}

int LuaSerializer::l_disconnect(lua_State *l)
{
	LUA_DEBUG_START(l)

	lua_Debug ar;
    lua_getstack(l, 0, &ar);
	lua_getinfo(l, "S", &ar);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializer");
	lua_getfield(l, -1, ar.short_src);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 2);
		LUA_DEBUG_END(l, 0);
		return 0;
	}

	lua_pop(l, 1);

	lua_pushstring(l, ar.short_src);
	lua_pushnil(l);
	lua_rawset(l, -3);

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0)

	return 0;
}

template <> const char *LuaObject<LuaSerializer>::s_type = "Serializer";
template <> const char *LuaObject<LuaSerializer>::s_inherit = NULL;

template <> const luaL_reg LuaObject<LuaSerializer>::s_methods[] = {
	{ "Connect",    LuaSerializer::l_connect    },
	{ "Disconnect", LuaSerializer::l_disconnect },
	{ 0, 0 }
};

template <> const luaL_reg LuaObject<LuaSerializer>::s_meta[] = {
	{ 0, 0 }
};
