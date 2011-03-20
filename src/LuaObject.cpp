#include "LuaObject.h"

static lid next_id = 0;
static std::map<lid, lpair> registry;

LuaObject::LuaObject(Object *o, const char *type)
{
	m_id = next_id++;
	assert(m_id < (lid)-1);

	Register(o, type);
}

void LuaObject::Register(Object *o, const char *type)
{
	lpair pair;
	pair.o = o;
	pair.type = type;

	registry[m_id] = pair;

	o->onDelete.connect(sigc::mem_fun(this, &LuaObject::Deregister));
}

void LuaObject::Deregister()
{
	registry.erase(m_id);
	m_id = -1;
}

void LuaObject::Deregister(lid id)
{
	registry.erase(id);
}

lpair LuaObject::Lookup(lid id)
{
	return registry[id];
}

static int object_gc (lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lid *idp = (lid*)lua_touserdata(l, 1);
	LuaObject::Deregister(*idp);
	return 0;
}

void LuaObject::CreateClass(const char *type, const luaL_reg methods[], const luaL_reg meta[])
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	// create table, attach methods to it, leave it on the stack
	luaL_openlib(l, type, methods, 0);

	// create the metatable, leave it on the stack
	luaL_newmetatable(l, type);
	// attach metamethods to it
	luaL_openlib(l, 0, meta, 0);

	// add a generic garbage collector
	lua_pushstring(l, "__gc");
	lua_pushcfunction(l, object_gc);
	lua_rawset(l, -3);

	// attach the metatable to __index
	lua_pushstring(l, "__index");
	lua_pushvalue(l, -3);
	lua_rawset(l, -3);

	// also attach it to __metatable
	lua_pushstring(l, "__metatable");
	lua_pushvalue(l, -3);
	lua_rawset(l, -3);

	// remove the metatable and the class table from the stack
	lua_pop(l, 2);
}

void LuaObject::PushToLua(const char *type)
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	lid *idp = (lid*)lua_newuserdata(l, sizeof(lid));
	*idp = m_id;

	luaL_getmetatable(l, type);
	lua_setmetatable(l, -2);
}
