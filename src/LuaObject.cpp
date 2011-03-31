#include "libs.h"
#include "LuaObject.h"
#include "LuaUtils.h"

#include <map>
#include <utility>

static lid next_id = 0;
static std::map<lid, LuaObjectBase*> registry;

LuaObjectBase::LuaObjectBase(DeleteEmitter *o, const char *type, bool wantdelete)
{
	m_object = o;
	m_type = type;
	m_wantDelete = wantdelete;

	m_id = ++next_id;
	assert(m_id);

	registry.insert(std::make_pair(m_id, this));

	m_deleteConnection = m_object->onDelete.connect(sigc::bind(sigc::ptr_fun(&LuaObjectBase::Deregister), this));

	lua_State *l = LuaManager::Instance()->GetLuaState();

	LUA_DEBUG_START(l);

	lid *idp = (lid*)lua_newuserdata(l, sizeof(lid));
	*idp = m_id;

	luaL_getmetatable(l, type);
	lua_setmetatable(l, -2);

	LUA_DEBUG_END(l, 1);
}

void LuaObjectBase::Deregister(LuaObjectBase *lo)
{
	lo->m_deleteConnection.disconnect();
	registry.erase(lo->m_id);
	if (lo->m_wantDelete) delete lo->m_object;
	delete lo;
}

LuaObjectBase *LuaObjectBase::Lookup(lid id)
{
	std::map<lid, LuaObjectBase*>::const_iterator i = registry.find(id);
	if (i == registry.end()) return NULL;
	return (*i).second;
}

int LuaObjectBase::GC(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lid *idp = (lid*)lua_touserdata(l, 1);
	LuaObjectBase *lo = Lookup(*idp);
	if (lo) Deregister(lo);
	return 0;
}

void LuaObjectBase::CreateClass(const char *type, const char *inherit, const luaL_reg methods[], const luaL_reg meta[])
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	LUA_DEBUG_START(l);

	// create table, attach methods to it, leave it on the stack
	luaL_register(l, type, methods);

	// create the metatable, leave it on the stack
	luaL_newmetatable(l, type);
	// attach metamethods to it
	luaL_register(l, 0, meta);

	// add a generic garbage collector
	lua_pushstring(l, "__gc");
	lua_pushcfunction(l, LuaObjectBase::GC);
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
		luaL_getmetatable(l, inherit);
		if (lua_isnil(l, -1))
			Error("Lua type '%s' can't inherit from unknown type '%s'", type, inherit);

		// attach it to the method table
		lua_setmetatable(l, -3);
	}

	// remove the metatable and the method table from the stack
	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}

DeleteEmitter *LuaObjectBase::GetFromLua(int index, const char *want_type)
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	LUA_DEBUG_START(l);

	luaL_checktype(l, index, LUA_TUSERDATA);

	lid *idp = (lid*)lua_touserdata(l, index);
	if (!idp)
		Error("Lua value on stack is of type userdata but has no userdata associated with it");

	LuaObjectBase *lo = LuaObjectBase::Lookup(*idp);
	if (!lo)
		Error("Lua object with id 0x%08x not found in registry", *idp);

	LUA_DEBUG_END(l, 0);

	if (!lo->Isa(want_type))
		Error("Lua object on stack has type %s which can not be used as type %s\n", lo->m_type, want_type);

	// found it
	return lo->m_object;
}

bool LuaObjectBase::Isa(const char *want_type) const
{
	lua_State *l = LuaManager::Instance()->GetLuaState();

	LUA_DEBUG_START(l);

	const char *current_type = this->m_type;
 
	// look for the type. we walk up the inheritance chain looking to see if
	// the passed object is a subclass of the wanted type
	while (strcmp(current_type, want_type) != 0) {
		// no match, up we go

		// get the method table for the current type
		lua_getfield(l, LUA_GLOBALSINDEX, current_type);

		// get its metatable
		if (!lua_getmetatable(l, -1)) {
			// not found means we've reached the base and can go no further
			lua_pop(l, 1);
			LUA_DEBUG_END(l, 0);
			return false;
		}

		// get the type this metatable belongs to 
		lua_getfield(l, -1, "type");
		current_type = lua_tostring(l, -1);

		lua_pop(l, 3);
	}

	LUA_DEBUG_END(l, 0);

	return true;
}

