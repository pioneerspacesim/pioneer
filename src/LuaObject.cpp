#include "libs.h"
#include "LuaObject.h"

#include <map>
#include <utility>

static lid next_id = 0;
static std::map<lid, LuaObject*> registry;

LuaObject::LuaObject(DeleteEmitter *o, const char *type, bool wantdelete)
{
	m_object = o;
	m_type = type;
	m_wantDelete = wantdelete;

	m_id = ++next_id;
	assert(m_id);

	registry.insert(std::make_pair(m_id, this));

	m_deleteConnection = m_object->onDelete.connect(sigc::bind(sigc::ptr_fun(&LuaObject::Deregister), this));

	lua_State *l = LuaManager::Instance()->GetLuaState();

	lid *idp = (lid*)lua_newuserdata(l, sizeof(lid));
	*idp = m_id;

	luaL_getmetatable(l, type);
	lua_setmetatable(l, -2);
}

void LuaObject::Deregister(LuaObject *lo)
{
	lo->m_deleteConnection.disconnect();
	registry.erase(lo->m_id);
	if (lo->m_wantDelete) delete lo->m_object;
	delete lo;
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
	LuaObject *lo = Lookup(*idp);
	if (lo) Deregister(lo);
	return 0;
}

void LuaObject::CreateClass(const char *type, const char *inherit, const luaL_reg methods[], const luaL_reg meta[])
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	// create table, attach methods to it, leave it on the stack
	luaL_register(l, type, methods);

	// create the metatable, leave it on the stack
	luaL_newmetatable(l, type);
	// attach metamethods to it
	luaL_register(l, 0, meta);

	// add a generic garbage collector
	lua_pushstring(l, "__gc");
	lua_pushcfunction(l, LuaObject::GC);
	lua_rawset(l, -3);

	// attach the method table to __index
	lua_pushstring(l, "__index");
	lua_pushvalue(l, -3);
	lua_rawset(l, -3);

	// add the type so we can walk the inheritance chain
	lua_pushstring(l, "type");
	lua_pushstring(l, type);
	lua_rawset(l, -3);

	// setup inheritance if wanted
	if (inherit) {
		// get the parent metatable
		luaL_getmetatable(l, inherit); // XXX handle not found

		// attach it to the method table
		lua_setmetatable(l, -3);
	}

	// remove the metatable and the method table from the stack
	lua_pop(l, 2);
}

DeleteEmitter *LuaObject::PullFromLua(const char *want_type)
{
	// XXX handle errors gracefully
	lua_State *l = LuaManager::Instance()->GetLuaState();

	luaL_checktype(l, 1, LUA_TUSERDATA);

	lid *idp = (lid*)lua_touserdata(l, 1);
	assert(idp);
	lua_remove(l, 1);

	LuaObject *lo = LuaObject::Lookup(*idp);
	assert(lo);                                    

	// XXX inherited type checks
	assert(strcmp(lo->m_type, want_type) == 0);

	return lo->m_object;
}
