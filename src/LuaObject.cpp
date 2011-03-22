#include "libs.h"
#include "LuaObject.h"

#include <map>
#include <utility>

static lid next_id = 0;
static std::map<lid, LuaObject*> registry;

void LuaObject::Register()
{
	m_id = next_id++;
	assert(m_id < (lid)-1);

	registry.insert(std::make_pair(m_id, this));

	m_deleteConnection = m_object->onDelete.connect(sigc::mem_fun(this, &LuaObject::Deregister));
}

void LuaObject::Deregister()
{
	m_deleteConnection.disconnect();
	registry.erase(m_id);
	delete this;
}

LuaObject *LuaObject::Lookup(lid id)
{
	std::map<lid, LuaObject*>::const_iterator i = registry.find(id);
	if (i == registry.end()) return NULL;
	return (*i).second;
}

int LuaObject::GC(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lid *idp = (lid*)lua_touserdata(l, 1);
	LuaObject::Lookup(*idp)->Deregister();
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
	lua_pushcfunction(l, LuaObject::GC);
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

void LuaObject::PushToLua() const
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	lid *idp = (lid*)lua_newuserdata(l, sizeof(lid));
	*idp = m_id;

	luaL_getmetatable(l, this->GetType());
	lua_setmetatable(l, -2);
}

DeleteEmitter *LuaObject::PullFromLua(lua_State *l, const char *want_type)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lid *idp = (lid*)lua_touserdata(l, 1);
	LuaObject *lo = LuaObject::Lookup(*idp);

	// XXX handle gracefully
	assert(lo);                                    
	assert(strcmp(lo->GetType(), want_type) == 0);

	return lo->GetObject();
}
