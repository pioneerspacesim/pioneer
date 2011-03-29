#include "LuaSerializer.h"
#include "LuaObject.h"

// every module can save one object. that will usually be a table.  we call
// each serializer in turn and capture its return value we build a table like
// so:
// {
//   'Assassination.lua' = { ... },
//   'DeliverPackage.lua' = { ... },
//   ...
// }
// entire table then gets pickled and handed to the writer
//
// on load, we unpickle the table then call the registered unserialize
// function for each module with its table
//
// we keep a copy of this table around. next time we save we overwrite the
// each individual module's data. that way if a player loads a game with dat
// for a module that is not currently loaded, we don't lose its data in the
// next save
//
// pickler can handle simple types (boolean, number, string) and will drill
// down into tables. it can do userdata for a specific set of types - Body and
// its kids and SBodyPath. anything else will cause a lua error
//
// pickle format is newline-seperated. each line begins with a type value,
// followed by data for that type as follows
//   f - number (float). stringified number on next line
//   b - boolean. N is 0 or 1, denoting false or true
//   s - string. stringified length number on next line, then string bytes follow immediately after, then newline
//   t - table. table contents are more pickled stuff (ie recursive)
//   n - end of table
//   u - userdata. type is on next line, followed by data
//     Body      - data is a single stringified number for Serializer::LookupBody
//     SBodyPath - data is four stringified numbers, newline separated

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
	lua_State *l = LuaManager::Instance()->GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "PiSaveData");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiSaveData");
	}

	lua_getfield(l, LUA_REGISTRYINDEX, "PiSerializer");
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		lua_newtable(l);
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializer");
	}

	// outer save table is now on top of the stack. call each module in turn
	lua_pushnil(l);
	while (lua_next(l, -2) != 0) {
		LUA_DEBUG_START(l)
		lua_pushinteger(l, 1);
		lua_gettable(l, -2);
		lua_call(l, 0, 1);
		lua_pushvalue(l, -3);
		lua_insert(l, -2);
		lua_settable(l, -6);
		lua_pop(l, 1);
		LUA_DEBUG_END(l, 0)
	}

	lua_pop(l, 1);

	// we're left with PiSaveData on the stack, all up to date. pickle time
	assert(0);

	LUA_DEBUG_END(l, 0);
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
		lua_pushvalue(l, -1);
		lua_setfield(l, LUA_REGISTRYINDEX, "PiSerializer");
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
