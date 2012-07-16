#include "libs.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "Pi.h"

#include <map>
#include <utility>

/*
 * Namespace: Object
 *
 * Provides core methods to all engine objects.
 *
 * <Object> is not a true class but does provide a few methods that are
 * applied to every Pioneer engine object that gets exposed to the Lua
 * environment.
 *
 *
 * Method: exists
 *
 * Determines if the engine object underpinning the Lua object still exists in
 * the engine.
 *
 * > exists = object:exists()
 *
 * It is possible for a Pioneer engine object to be deleted while a Lua script
 * is holding a reference to it. In this case, the Lua object is simply a
 * shell, with no internals, and any attempt to use it will result in a Lua
 * error. Calling <exists> allows a script to determine if the object is still
 * valid before it uses it.
 *
 * Modules that carry their own state or do things out of the normal event
 * flow need to consider this. The documentation for <Timer> contains a more
 * concrete example of this.
 *
 * Returns:
 *
 *   exists - true if the object is valid, false otherwise
 *
 * Example:
 *
 * > if not ship:exists() then return
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 *
 *
 * Method: isa
 *
 * Determines if a object is of or inherits from a given class.
 *
 * > isa = object:isa(classname)
 *
 * The object model used in Pioneer's Lua environment is a classical
 * single-inheritance system. <isa> operates much like similarly-named methods
 * in other languages; it tells you if the object has the named class
 * somewhere in its inheritence hierarchy.
 *
 * Parameters:
 *
 *   classname - the name of the class to check against the object
 *
 * Returns:
 *
 *   isa - true if the object inherits from the named class, false otherwise
 *
 * Example:
 *
 * > if body:isa("Ship") then
 * >     body:AIKill(Game.player)
 * > end
 *
 * Availability:
 *
 *   alpha 10
 *
 * Status:
 *
 *   stable
 */

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

		// XXX atexit is not a very nice way to deal with this in C++
		atexit(_teardown);

		instantiated = true;
	}
}

void LuaObjectBase::Deregister(LuaObjectBase *lo)
{
	lo->m_deleteConnection.disconnect();
	registry->erase(lo->m_id);

	lua_State *l = Pi::luaManager->GetLuaState();

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
	lid *idp = static_cast<lid*>(lua_touserdata(l, 1));
	LuaObjectBase *lo = Lookup(*idp);
	lua_pushboolean(l, lo != 0);
	return 1;
}

int LuaObjectBase::l_isa(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lid *idp = static_cast<lid*>(lua_touserdata(l, 1));

	LuaObjectBase *lo = Lookup(*idp);
	if (!lo) {
		// luaL_error format codes are very limited (can't handle width or fill specifiers),
		// so we use snprintf here to do the real formatting
		char objectCode[16];
		snprintf(objectCode, sizeof(objectCode), "0x%08x", *idp);
		luaL_error(l, "Lua object with id %s not found in registry", objectCode);
	}

	lua_pushboolean(l, lo->Isa(luaL_checkstring(l, 2)));
	return 1;
}

int LuaObjectBase::l_gc(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lid *idp = static_cast<lid*>(lua_touserdata(l, 1));
	LuaObjectBase *lo = Lookup(*idp);
	if (lo) Deregister(lo);
	return 0;
}

int LuaObjectBase::l_tostring(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	lua_getmetatable(l, 1);
	lua_pushstring(l, "type");
	lua_rawget(l, -2);
	lua_pushfstring(l, "userdata [%s]: %p", lua_tostring(l, -1), lua_topointer(l, 1));
	return 1;
}

static int dispatch_index(lua_State *l)
{
	// userdata are typed, tables are not
	bool typeless = lua_istable(l, 1);
	assert(typeless || lua_isuserdata(l, 1));

	// ensure we have enough stack space
	luaL_checkstack(l, 8, 0);

	// each type has a global method table, which we need access to
	lua_rawgeti(l, LUA_REGISTRYINDEX, LUA_RIDX_GLOBALS);

	// everything we need is in the metatable, so lets start with that
	lua_getmetatable(l, 1);             // object, key, globals, metatable

	// loop until we find what we're looking for or we run out of metatables
	while (!lua_isnil(l, -1)) {

		// get the method table
		if (typeless) {
			// the object is the method table
			lua_pushvalue(l, 1);            // object, key, globals, metatable, method table
		}

		else {
			// get the object type from the metatable and use it to look up
			// the method table
			lua_pushstring(l, "type");
			lua_rawget(l, -2);              // object, key, globals, metatable, type

			lua_rawget(l, -3);              // object, key, globals, metatable, method table
		}

		lua_pushvalue(l, 2);
		lua_rawget(l, -2);                  // object, key, globals, metatable, method table, method

		// found something, return it
		if (!lua_isnil(l, -1))
			return 1;
		lua_pop(l, 1);                      // object, key, globals, metatable, method table

		// didn't find a method, so now we go looking for an attribute handler
		lua_pushstring(l, (std::string("__attribute_")+lua_tostring(l, 2)).c_str());
		lua_rawget(l, -2);                  // object, key, globals, metatable, method table, method

		// found something, return it
		if (!lua_isnil(l, -1)) {
			// found something. since its likely a regular attribute lookup and not a
			// method call we have to do the call ourselves
			if (lua_isfunction(l, -1)) {
				lua_pushvalue(l, 1);
				pi_lua_protected_call(l, 1, 1);
				return 1;
			}

			// for the odd case where someone has set __attribute_foo to a
			// non-function value
			return 1;
		}

		lua_pop(l, 2);                      // object, key, globals, metatable

		// didn't find anything. if the object has a parent object then we look
		// there instead
		lua_pushstring(l, "parent");
		lua_rawget(l, -2);                  // object, key, globals, metatable, parent type

		// not found means we have no parents and we can't search any further
		if (lua_isnil(l, -1))
			break;

		// clean up the stack
		lua_remove(l, -2);                  // object, key, globals, parent type

		// get the parent metatable
		lua_rawget(l, LUA_REGISTRYINDEX);   // object, key, globals, parent metatable
	}

	luaL_error(l, "unable to resolve method or attribute '%s'", lua_tostring(l, 2));
	return 0;
}

void LuaObjectBase::CreateObject(const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta)
{
	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);

	// create "object"
	lua_newtable(l);
	if (methods) luaL_setfuncs(l, methods, 0);

	// add attributes
	if (attrs) {
		for (const luaL_Reg *attr = attrs; attr->name; attr++) {
			lua_pushstring(l, (std::string("__attribute_")+attr->name).c_str());
			lua_pushcfunction(l, attr->func);
			lua_rawset(l, -3);
		}
	}

	// create metatable for it
	lua_newtable(l);
	if (meta) luaL_setfuncs(l, meta, 0);

	// index function
	lua_pushstring(l, "__index");
	lua_pushcfunction(l, dispatch_index);
	lua_rawset(l, -3);

	// apply the metatable
	lua_setmetatable(l, -2);

	// leave the finished object on the stack

	LUA_DEBUG_END(l, 1);
}

void LuaObjectBase::CreateClass(const char *type, const char *parent, const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta)
{
	assert(type);

	lua_State *l = Pi::luaManager->GetLuaState();

	_instantiate();

	LUA_DEBUG_START(l);

	// create the object registry if it doesn't already exist. this is the
	// best place we have to do this since classes will always be registered
	// before any objects actually turn up
	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	if (lua_isnil(l, -1)) {
		// create the LuaObjectRegistry table
		lua_newtable(l);

		// configure the registry to use weak values
		lua_newtable(l);
		lua_pushstring(l, "__mode");
		lua_pushstring(l, "v");
		lua_rawset(l, -3);
		lua_setmetatable(l, -2);

		lua_setfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	}
	lua_pop(l, 1);

	// create table, attach methods to it, leave it on the stack
	lua_newtable(l);
    if (methods) luaL_setfuncs(l, methods, 0);

	// add attributes
	if (attrs) {
		for (const luaL_Reg *attr = attrs; attr->name; attr++) {
			lua_pushstring(l, (std::string("__attribute_")+attr->name).c_str());
			lua_pushcfunction(l, attr->func);
			lua_rawset(l, -3);
		}
	}

	// add the exists method
	lua_pushstring(l, "exists");
	lua_pushcfunction(l, LuaObjectBase::l_exists);
	lua_rawset(l, -3);

	// add the isa method
	lua_pushstring(l, "isa");
	lua_pushcfunction(l, LuaObjectBase::l_isa);
	lua_rawset(l, -3);

	// publish the method table as a global (and pop it from the stack)
	lua_setglobal(l, type);

	// create the metatable, leave it on the stack
	luaL_newmetatable(l, type);

	// default tostring method. setting before setting up user-supplied
	// metamethods because they might override it
	lua_pushstring(l, "__tostring");
	lua_pushcfunction(l, LuaObjectBase::l_tostring);
	lua_rawset(l, -3);

	// attach supplied metamethods
	if (meta) luaL_setfuncs(l, meta, 0);

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

	// pop the metatable
	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}

bool LuaObjectBase::PushRegistered(DeleteEmitter *o)
{
	assert(instantiated);

	lua_State *l = Pi::luaManager->GetLuaState();

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

	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	assert(lua_istable(l, -1));

	lid *idp = static_cast<lid*>(lua_newuserdata(l, sizeof(lid)));
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

	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);

	if (lua_type(l, index) != LUA_TUSERDATA)
		return NULL;

	lid *idp = static_cast<lid*>(lua_touserdata(l, index));
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

	lua_State *l = Pi::luaManager->GetLuaState();

	LUA_DEBUG_START(l);

	luaL_checktype(l, index, LUA_TUSERDATA);

	lid *idp = static_cast<lid*>(lua_touserdata(l, index));
	if (!idp)
		luaL_error(l, "Lua value on stack is of type userdata but has no userdata associated with it");

	LuaObjectBase *lo = LuaObjectBase::Lookup(*idp);
	if (!lo) {
		// luaL_error format codes are very limited (can't handle width or fill specifiers),
		// so we use snprintf here to do the real formatting
		char objectCode[16];
		snprintf(objectCode, sizeof(objectCode), "0x%08x", *idp);
		luaL_error(l, "Lua object with id %s not found in registry", objectCode);
	}

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

	lua_State *l = Pi::luaManager->GetLuaState();

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
