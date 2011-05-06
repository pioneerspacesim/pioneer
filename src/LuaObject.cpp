#include "libs.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"

#include <map>
#include <utility>

// since LuaManager is a singleton, these must be heap-allocated. If they're
// stack-allocated, they will be torn down at program shutdown before the
// singleton is. This will cause LuaObject to crash during garbage collection
static bool instantiated = false;
static lid next_id = 0;

static std::map<lid, LuaObjectBase*> *registry;
static std::map< std::string, std::map<std::string,PromotionTest> > *promotions;

static void _teardown() {
	delete registry;
	delete promotions;
}

static inline void _instantiate() {
	if (!instantiated) {
		registry = new std::map<lid, LuaObjectBase*>;
		promotions = new std::map< std::string, std::map<std::string,PromotionTest> >;

		// XXX I want to do this, because its good to clean up at end of
		// program if you can. doing it reintroduces the crash though, as this
		// gets called before all our objects are destroyed. I'm not sure what
		// the solution is at this time
		//atexit(_teardown);

		instantiated = true;
	}
}

void LuaObjectBase::Deregister(LuaObjectBase *lo)
{
	lo->m_deleteConnection.disconnect();
	registry->erase(lo->m_id);

	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	assert(lua_istable(l, -1));

	lua_pushlightuserdata(l, lo->m_object);
	lua_pushnil(l);
	lua_settable(l, -3);

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	lo->Release(lo->m_object);

	if (lo->m_wantDelete) delete lo->m_object;
	delete lo;
}

LuaObjectBase *LuaObjectBase::Lookup(lid id)
{
	std::map<lid, LuaObjectBase*>::const_iterator i = registry->find(id);
	if (i == registry->end()) return NULL;
	return (*i).second;
}

int LuaObjectBase::l_exists(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lid *idp = (lid*)lua_touserdata(l, 1);
	LuaObjectBase *lo = Lookup(*idp);
	lua_pushboolean(l, lo != 0);
	return 1;
}

int LuaObjectBase::l_isa(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lid *idp = (lid*)lua_touserdata(l, 1);

	LuaObjectBase *lo = Lookup(*idp);
	if (!lo)
		luaL_error(l, "Lua object with id 0x%08x not found in registry", *idp);

	lua_pushboolean(l, lo->Isa(luaL_checkstring(l, 2)));
	return 1;
}

int LuaObjectBase::l_gc(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lid *idp = (lid*)lua_touserdata(l, 1);
	LuaObjectBase *lo = Lookup(*idp);
	if (lo) Deregister(lo);
	return 0;
}

static int dispatch_index(lua_State *l)
{
	// if its a table then they're peeking inside the method table directly
	// (non-object call, curreying, etc) and we should just mimic the standard
	// lookup behaviour
	if (lua_istable(l, 1)) {
		lua_rawget(l, 1);
		return 1;
	}

	// sanity check. it should be a userdatum
	assert(lua_isuserdata(l, 1));

	// everything we need is in the metatable, so lets start with that
	lua_getmetatable(l, 1);             // object, key, metatable

	// loop until we find what we're looking for or we run out of metatables
	while (!lua_isnil(l, -1)) {

		// first is method lookup. we get the object type from the metatable and
		// use it to look up the method table and from there, the method itself
		lua_pushstring(l, "type");
		lua_rawget(l, -2);                  // object, key, metatable, type

		lua_rawget(l, LUA_GLOBALSINDEX);    // object, key, metatable, method table

		lua_pushvalue(l, 2);
		lua_rawget(l, -2);                  // object, key, metatable, method table, method
    
		// found something, return it
		if (!lua_isnil(l, -1))
			return 1;

		lua_pop(l, 2);                      // object, key, metatable

		// didn't find a method, so now we go looking for an attribute handler in
		// the attribute table
		lua_pushstring(l, "attrs");
		lua_rawget(l, -2);                  // object, key, metatable, attr table

		if (lua_istable(l, -1)) {
			lua_pushvalue(l, 2);
			lua_rawget(l, -2);              // object, key, metatable, attr table, attr handler

			// found something. since its likely a regular attribute lookup and not a
			// method call we have to do the call ourselves
			if (lua_isfunction(l, -1)) {
				lua_pushvalue(l, 1);
				lua_call(l, 1, 1);
				return 1;
			}

			lua_pop(l, 2);                  // object, key, metatable
		}
		else
			lua_pop(l, 1);                  // object, key, metatable

		// didn't find anything. if the object has a parent object then we look
		// there instead
		lua_pushstring(l, "parent");
		lua_rawget(l, -2);                  // object, key, metatable, parent type

		// not found means we have no parents and we can't search any further
		if (lua_isnil(l, -1))
			break;

		// clean up the stack
		lua_remove(l, -2);                  // object, key, parent type

		// get the parent metatable
		lua_rawget(l, LUA_REGISTRYINDEX);   // object, key, parent metatable
	}

	luaL_error(l, "unable to resolve method or attribute '%s'", lua_tostring(l, 2));
	return 0;
}

static const luaL_reg no_methods[] = {
	{ 0, 0 }
};

void LuaObjectBase::CreateClass(const char *type, const char *parent, const luaL_reg *methods, const luaL_reg *attrs, const luaL_reg *meta)
{
	assert(type);

	lua_State *l = Pi::luaManager.GetLuaState();

	_instantiate();

	LUA_DEBUG_START(l);

	// create the object registry if it doesn't already exist. this is the
	// best place we have to do this since classes will always be registered
	// before any objects actually turn up
	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	if (lua_isnil(l, -1)) {

		// setup a metatable for weak values
		lua_newtable(l);
		lua_pushstring(l, "__mode");
		lua_pushstring(l, "v");
		lua_rawset(l, -3);

		// and the table proper
		lua_newtable(l);
		lua_setmetatable(l, -2);
		lua_setfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	}
	lua_pop(l, 1);

	// create table, attach methods to it, leave it on the stack
	luaL_register(l, type, methods ? methods : no_methods);

	// add the exists method
	lua_pushstring(l, "exists");
	lua_pushcfunction(l, LuaObjectBase::l_exists);
	lua_rawset(l, -3);

	// add the isa method
	lua_pushstring(l, "isa");
	lua_pushcfunction(l, LuaObjectBase::l_isa);
	lua_rawset(l, -3);

	// create the metatable, leave it on the stack
	luaL_newmetatable(l, type);
	// attach metamethods to it
	if (meta) luaL_register(l, 0, meta);

	// add a generic garbage collector
	lua_pushstring(l, "__gc");
	lua_pushcfunction(l, LuaObjectBase::l_gc);
	lua_rawset(l, -3);

	// setup a custom index function. this thing handles all the magic of
	// finding the right function or attribute and walking the inheritance
	// hierarchy as necessary
	lua_pushstring(l, "__index");
	lua_pushcfunction(l, dispatch_index);
	lua_rawset(l, -3);

	// record the type in the metatable so we know what we're looking at for
	// the inheritance walk
	lua_pushstring(l, "type");
	lua_pushstring(l, type);
	lua_rawset(l, -3);

	// if we're inheriting, record the name of the base type
	if (parent) {
		lua_pushstring(l, "parent");
		lua_pushstring(l, parent);
		lua_rawset(l, -3);
	}

	// if they passed attributes hook them up too
	if (attrs) {
		lua_pushstring(l, "attrs");
		
		lua_newtable(l);
		luaL_register(l, 0, attrs);

		lua_rawset(l, -3);

	}

	// remove the metatable and the method table from the stack
	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);
}

bool LuaObjectBase::PushRegistered(DeleteEmitter *o)
{
	assert(instantiated);

	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	if (!o) {
		lua_pushnil(l);
		return true;
	}

	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	assert(lua_istable(l, -1));

	lua_pushlightuserdata(l, o);
	lua_gettable(l, -2);

	if (lua_isuserdata(l, -1)) {
		lua_insert(l, -2);
		lua_pop(l, 1);

		LUA_DEBUG_END(l, 1);

		return true;
	}
	assert(lua_isnil(l, -1));

	lua_pop(l, 2);

	LUA_DEBUG_END(l, 0);

	return false;
}

void LuaObjectBase::Push(LuaObjectBase *lo, bool wantdelete)
{
	assert(instantiated);

	lo->m_wantDelete = wantdelete;

	lo->m_id = ++next_id;
	assert(lo->m_id);

	bool have_promotions = true;
	bool tried_promote = false;
	
	while (have_promotions && !tried_promote) {
		std::map< std::string, std::map<std::string,PromotionTest> >::const_iterator base_iter = promotions->find(lo->m_type);
		if (base_iter != promotions->end()) {
			tried_promote = true;

			for (
				std::map<std::string,PromotionTest>::const_iterator target_iter = (*base_iter).second.begin();
				target_iter != (*base_iter).second.end(); 
				target_iter++)
			{
				if ((*target_iter).second(lo->m_object)) {
					lo->m_type = (*target_iter).first.c_str();
					tried_promote = false;
				}
			}

			assert(lo->Isa((*base_iter).first.c_str()));
		}
		else
			have_promotions = false;
	}

	lo->Acquire(lo->m_object);

	lo->m_deleteConnection = lo->m_object->onDelete.connect(sigc::bind(sigc::ptr_fun(&LuaObjectBase::Deregister), lo));

	registry->insert(std::make_pair(lo->m_id, lo));

	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	assert(lua_istable(l, -1));

	lid *idp = (lid*)lua_newuserdata(l, sizeof(lid));
	*idp = lo->m_id;

	luaL_getmetatable(l, lo->m_type);
	lua_setmetatable(l, -2);

	lua_pushlightuserdata(l, lo->m_object);
	lua_pushvalue(l, -2);
	lua_settable(l, -4);

	lua_insert(l, -2);
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 1);
}

DeleteEmitter *LuaObjectBase::CheckFromLua(int index, const char *type)
{
	assert(instantiated);

	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	if (lua_type(l, index) != LUA_TUSERDATA)
		return NULL;

	lid *idp = (lid*)lua_touserdata(l, index);
	if (!idp)
		return NULL;

	LuaObjectBase *lo = LuaObjectBase::Lookup(*idp);
	if (!lo)
		return NULL;

	LUA_DEBUG_END(l, 0);

	if (!lo->Isa(type))
		return NULL;

	// found it
	return lo->m_object;
}

DeleteEmitter *LuaObjectBase::GetFromLua(int index, const char *type)
{
	assert(instantiated);

	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	luaL_checktype(l, index, LUA_TUSERDATA);

	lid *idp = (lid*)lua_touserdata(l, index);
	if (!idp)
		luaL_error(l, "Lua value on stack is of type userdata but has no userdata associated with it");

	LuaObjectBase *lo = LuaObjectBase::Lookup(*idp);
	if (!lo)
		luaL_error(l, "Lua object with id 0x%08x not found in registry", *idp);

	LUA_DEBUG_END(l, 0);

	if (!lo->Isa(type))
		luaL_error(l, "Lua object on stack has type %s which can not be used as type %s\n", lo->m_type, type);

	// found it
	return lo->m_object;
}

bool LuaObjectBase::Isa(const char *base) const
{
	// fast path
	if (strcmp(m_type, base) == 0)
		return true;

	assert(instantiated);

	lua_State *l = Pi::luaManager.GetLuaState();

	LUA_DEBUG_START(l);

	lua_pushstring(l, m_type);
	while (strcmp(lua_tostring(l, -1), base) != 0) {
		// get the metatable for the current type
		lua_rawget(l, LUA_REGISTRYINDEX);

		// get the name of the parent type
		lua_pushstring(l, "parent");
		lua_rawget(l, -2);

		// if it doesn't have a parent then we can go no further
		if (lua_isnil(l, -1)) {
			lua_pop(l, 2);
			LUA_DEBUG_END(l, 0);
			return false;
		}

		lua_remove(l, -2);
	}
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	return true;
}

void LuaObjectBase::RegisterPromotion(const char *base_type, const char *target_type, PromotionTest test_fn)
{
	(*promotions)[base_type][target_type] = test_fn;
}
