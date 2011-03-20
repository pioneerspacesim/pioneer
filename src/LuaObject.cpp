#include "LuaObject.h"

static lid next_id = 0;
static std::map<lid, Object*> registry;

LuaObject::LuaObject(Object *o)
{
	m_id = next_id++;
	assert(m_id < (lid)-1);

	Register(o);
}

LuaObject::~LuaObject()
{
	if (m_id == (lid)-1) return;
	//Deregister();
}

void LuaObject::Register(Object *o)
{
	registry[m_id] = o;
	o->onDelete.connect(sigc::mem_fun(this, &LuaObject::Deregister));
}

void LuaObject::Deregister()
{
	registry.erase(m_id);
	m_id = -1;
}

Object *LuaObject::Lookup(lid id)
{
	return registry[id];
}

void LuaObject::CreateClass(const char *name, const luaL_reg methods[], const luaL_reg meta[])
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	// create table, attach methods to it, leave it on the stack
	luaL_openlib(l, name, methods, 0);

	// create the metatable, leave it on the stack
	luaL_newmetatable(l, name);
	// attach metamethods to it
	luaL_openlib(l, 0, meta, 0);

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

void LuaObject::PushToLua(const char *name)
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	lid *idp = (lid*)lua_newuserdata(l, sizeof(lid));
	*idp = m_id;

	luaL_getmetatable(l, name);
	lua_setmetatable(l, -2);
}
