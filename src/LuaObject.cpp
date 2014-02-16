// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "LuaObject.h"
#include "LuaUtils.h"
#include "PropertiedObject.h"
#include "PropertyMap.h"

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

static std::map< std::string, std::map<std::string,PromotionTest> > *promotions;

static void _teardown() {
	delete promotions;
}

static inline void _instantiate() {
	if (!instantiated) {
		promotions = new std::map< std::string, std::map<std::string,PromotionTest> >;

		// XXX atexit is not a very nice way to deal with this in C++
		atexit(_teardown);

		instantiated = true;
	}
}

int LuaObjectBase::l_exists(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	LuaObjectBase *lo = static_cast<LuaObjectBase*>(lua_touserdata(l, 1));
	lua_pushboolean(l, lo->GetObject() != 0);
	return 1;
}

int LuaObjectBase::l_hasprop(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	luaL_checktype(l, 2, LUA_TSTRING);
	lua_getuservalue(l, 1);

	if (lua_isnil(l, -1)) { // Doesn't have properties
		lua_pushboolean(l, false);
	} else {
		lua_pushvalue(l, 2);
		lua_gettable(l, -2);
		// We consider that a value isn't set if it is nil
		lua_pushboolean(l, !lua_isnil(l, -1));
	}
	return 1;
}

int LuaObjectBase::l_setprop(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	const std::string key(luaL_checkstring(l, 2));

	int isnum;
	double vn = lua_tonumberx(l, 3, &isnum);
	std::string vs;
	if (!isnum)
		vs = luaL_checkstring(l, 3);

	// quick check to make sure this object actually has properties
	// before we go diving through the stack etc
	lua_getuservalue(l, 1);
	if (lua_isnil(l, -1))
		return luaL_error(l, "Object has no property map");

	LuaObjectBase *lo = static_cast<LuaObjectBase*>(lua_touserdata(l, 1));
	LuaWrappable *o = lo->GetObject();
	if (!o)
		return luaL_error(l, "Object is no longer valid");

	PropertiedObject *po = dynamic_cast<PropertiedObject*>(o);
	assert(po);

	if (isnum)
		po->Properties().Set(key, vn);
	else
		po->Properties().Set(key, vs);

	return 0;
}

int LuaObjectBase::l_isa(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	LuaObjectBase *lo = static_cast<LuaObjectBase*>(lua_touserdata(l, 1));
	if (!lo->GetObject())
		return luaL_error(l, "Object is no longer valid");

	lua_pushboolean(l, lo->Isa(luaL_checkstring(l, 2)));
	return 1;
}

int LuaObjectBase::l_gc(lua_State *l)
{
	luaL_checktype(l, 1, LUA_TUSERDATA);
	LuaObjectBase *lo = static_cast<LuaObjectBase*>(lua_touserdata(l, 1));

	Deregister(lo);

	lo->~LuaObjectBase();

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

// takes metatable on top of stack
// if there's a parent, leaves next metatable, method on stack
// if there's no parent, leaves nil, method on stack
static void get_next_method_table(lua_State *l)
{
	LUA_DEBUG_START(l);

	// get the type from the table
	lua_pushstring(l, "type");
	lua_rawget(l, -2);                 // object, metatable, type

	const std::string type(lua_tostring(l, -1));
	lua_pop(l, 1);                     // object, metatable
	pi_lua_split_table_path(l, type);  // object, metatable, "global" table, leaf type name
	lua_rawget(l, -2);                 // object, metatable, "global" table, method table
	lua_remove(l, -2);                 // object, metatable, method table

	// see if the metatable has a parent
	lua_pushstring(l, "parent");
	lua_rawget(l, -3);                 // object, metatable, method table, parent type

	// it does, lets fetch it
	if (!lua_isnil(l, -1)) {
		lua_rawget(l, LUA_REGISTRYINDEX); // object, metatable, method table, parent metatable
		lua_replace(l, -3);               // object, parent metatable, method table
		LUA_DEBUG_END(l, 1);
		return;
	}

	// no parent
	                    // object, metatable, method table, nil
	lua_replace(l, -3); // object, nil, method table

	LUA_DEBUG_END(l, 1);
}

// takes table, name on top of stack
// if found, returns true, leaves item to return to lua on top of stack
// if not found, returns false
static bool get_method_or_attr(lua_State *l)
{
	LUA_DEBUG_START(l);

	// lookup wanted thing
	lua_pushvalue(l, -1);
	lua_rawget(l, -3);

	// found something, return it
	if (!lua_isnil(l, -1)) {
		LUA_DEBUG_END(l, 1);
		return true;
	}
	lua_pop(l, 1);

	// didn't find a method, so now we go looking for an attribute handler
	lua_pushstring(l, (std::string("__attribute_")+lua_tostring(l, -1)).c_str());
	lua_rawget(l, -3);

	// found something, return it
	if (!lua_isnil(l, -1)) {
		// found something. since its likely a regular attribute lookup and not a
		// method call we have to do the call ourselves
		if (lua_isfunction(l, -1)) {
			lua_pushvalue(l, 1);
			pi_lua_protected_call(l, 1, 1);
			LUA_DEBUG_END(l, 1);
			return true;
		}

		// for the odd case where someone has set __attribute_foo to a
		// non-function value
		LUA_DEBUG_END(l, 1);
		return true;
	}
	lua_pop(l, 1);

	// not found
	LUA_DEBUG_END(l, 0);
	return false;
}

int LuaObjectBase::l_dispatch_index(lua_State *l)
{
	// userdata are typed, tables are not
	bool typeless = lua_istable(l, 1);
	assert(typeless || lua_isuserdata(l, 1));

	// typeless objects have no parents, and are their own method table, so
	// this is easy
	if (typeless) {
		if (get_method_or_attr(l))
			return 1;
	}

	// normal userdata object
	else {

		// first check properties. we don't need to drill through lua if the
		// property is already available
		lua_getuservalue(l, 1);
		if (!lua_isnil(l, -1)) {
			lua_pushvalue(l, 2);
			if (get_method_or_attr(l))
				return 1;
			lua_pop(l, 1);
		}
		lua_pop(l, 1);

		lua_getmetatable(l, 1);
		while (1) {
			get_next_method_table(l);

			lua_pushvalue(l, 2);
			if (get_method_or_attr(l))
				return 1;

			// not found. remove name copy and method table
			lua_pop(l, 2);

			// if there's no parent metatable, get out
			if (lua_isnil(l, -1))
				break;
		}
	}

	luaL_error(l, "unable to resolve method or attribute '%s'", lua_tostring(l, 2));
	return 0;
}

static void get_names_from_table(lua_State *l, std::vector<std::string> &names, const std::string &prefix, bool methodsOnly)
{
	lua_pushnil(l);
	while (lua_next(l, -2)) {

		// only include string keys. the . syntax doesn't work for anything
		// else
		if (lua_type(l, -2) != LUA_TSTRING) {
			lua_pop(l, 1);
			continue;
		}

		// only include callable things if requested
		if (methodsOnly && lua_type(l, -1) != LUA_TFUNCTION) {
			lua_pop(l, 1);
			continue;
		}

		std::string name(lua_tostring(l, -2));

		// strip off magic attribute prefix
		if (name.substr(0, 12) == "__attribute_")
			name = name.substr(12);

		// anything else starting with an underscore is hidden
		else if (name[0] == '_') {
			lua_pop(l, 1);
			continue;
		}

		if (name.substr(0, prefix.size()) == prefix)
			names.push_back(name.substr(prefix.size()));

		lua_pop(l, 1);
	}
}

void LuaObjectBase::GetNames(std::vector<std::string> &names, const std::string &prefix, bool methodsOnly)
{
	// never show hidden names
	if (prefix[0] == '_')
		return;

	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	// work out if/how we can deal with the value
	bool typeless;
	if (lua_istable(l, -1))
		// we can always look into tables
		typeless = true;

	else if (lua_isuserdata(l, -1)) {
		// two known types of userdata
		// - LuaObject, metatable has a "type" field
		// - RO table proxy, no type
		lua_getmetatable(l, -1);
		lua_getfield(l, -1, "type");
		typeless = lua_isnil(l, -1);
		lua_pop(l, 2);
	}

	else
		// not a table or userdata, nothing to do
		// XXX if it has a __index metatable entry maybe we can do more?
		return;

	if (typeless) {
		// Check the metatable indexes
		lua_pushvalue(l, -1);
		while(lua_getmetatable(l, -1)) {
			lua_pushstring(l, "__index");
			lua_gettable(l, -2);

			// Replace the previous table to keep a stable stack size.
			lua_copy(l, -1, -3);
			lua_pop(l, 2);

			if (lua_istable(l, -1))
				get_names_from_table(l, names, prefix, methodsOnly);
			else
				break;
		}
		lua_pop(l, 1);

		if (lua_istable(l, -1))
			get_names_from_table(l, names, prefix, methodsOnly);

		return;
	}

	// properties
	if (!methodsOnly) {
		lua_getuservalue(l, -1);
		if (!lua_isnil(l, -1))
			get_names_from_table(l, names, prefix, false);
		lua_pop(l, 1);
	}

	lua_getmetatable(l, -1);
	while (1) {
		get_next_method_table(l);
		get_names_from_table(l, names, prefix, methodsOnly);

		lua_pop(l, 1);

		if (lua_isnil(l, -1))
			break;
	}

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
}

static int secure_trampoline(lua_State *l)
{
	// walk the stack
	// pass through any C functions
	// if we reach a non-C function, then check whether it's trusted and we're done
	// (note: trusted defaults to true because if the loop bottoms out then we've only gone through C functions)

	bool trusted = true;

	lua_Debug ar;
	int stack_pos = 1;
	while (lua_getstack(l, stack_pos, &ar) && lua_getinfo(l, "S", &ar)) {
		if (strcmp(ar.what, "C") != 0) {
			trusted = (strncmp(ar.source, "[T]", 3) == 0);
			break;
		}
		++stack_pos;
	}

	if (!trusted)
		luaL_error(l, "attempt to access protected method or attribute from untrusted script blocked");

	lua_CFunction fn = lua_tocfunction(l, lua_upvalueindex(1));
	return fn(l);
}

static void register_functions(lua_State *l, const luaL_Reg *methods, bool protect, const char *prefix)
{
	const size_t prefix_len = prefix ? strlen(prefix) : 0;
	for (const luaL_Reg *m = methods; m->name; m++) {
		if (prefix_len) {
			const size_t name_len = strlen(m->name);
			luaL_Buffer b;
			luaL_buffinitsize(l, &b, prefix_len + name_len);
			luaL_addlstring(&b, prefix, prefix_len);
			luaL_addlstring(&b, m->name, name_len);
			luaL_pushresult(&b);
		} else
			lua_pushstring(l, m->name);
		lua_pushcfunction(l, m->func);
		if (protect)
			lua_pushcclosure(l, secure_trampoline, 1);
		lua_rawset(l, -3);
	}
}

void LuaObjectBase::CreateObject(const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta, bool protect)
{
	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	// create "object"
	lua_newtable(l);

	// add methods
	if (methods) register_functions(l, methods, protect, "");

	// add attributes
	if (attrs) register_functions(l, attrs, protect, "__attribute_");

	// create metatable for it
	lua_newtable(l);
	if (meta) register_functions(l, meta, protect, "");

	// index function
	lua_pushstring(l, "__index");
	lua_pushcfunction(l, l_dispatch_index);
	if (protect)
		lua_pushcclosure(l, secure_trampoline, 1);
	lua_rawset(l, -3);

	// apply the metatable
	lua_setmetatable(l, -2);

	// leave the finished object on the stack

	LUA_DEBUG_END(l, 1);
}

void LuaObjectBase::CreateClass(const char *type, const char *parent, const luaL_Reg *methods, const luaL_Reg *attrs, const luaL_Reg *meta)
{
	assert(type);

	lua_State *l = Lua::manager->GetLuaState();

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

	// drill down to the proper "global" table to add the method table to
	pi_lua_split_table_path(l, type);

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

	// add the setprop and hasprop methods
	lua_pushstring(l, "setprop");
	lua_pushcfunction(l, LuaObjectBase::l_setprop);
	lua_rawset(l, -3);
	lua_pushstring(l, "hasprop");
	lua_pushcfunction(l, LuaObjectBase::l_hasprop);
	lua_rawset(l, -3);

	// publish the method table
	lua_rawset(l, -3);

	// remove the "global" table
	lua_pop(l, 1);

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
	lua_pushcfunction(l, l_dispatch_index);
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

bool LuaObjectBase::PushRegistered(LuaWrappable *o)
{
	assert(instantiated);

	lua_State *l = Lua::manager->GetLuaState();

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

void LuaObjectBase::Register(LuaObjectBase *lo)
{
	assert(instantiated);
	assert(lo->GetObject());

	bool have_promotions = true;
	bool tried_promote = false;

	while (have_promotions && !tried_promote) {
		std::map< std::string, std::map<std::string,PromotionTest> >::const_iterator base_iter = promotions->find(lo->m_type);
		if (base_iter != promotions->end()) {
			tried_promote = true;

			for (
				std::map<std::string,PromotionTest>::const_iterator target_iter = (*base_iter).second.begin();
				target_iter != (*base_iter).second.end();
				++target_iter)
			{
				if ((*target_iter).second(lo->GetObject())) {
					lo->m_type = (*target_iter).first.c_str();
					tried_promote = false;
				}
			}

			assert(lo->Isa((*base_iter).first.c_str()));
		}
		else
			have_promotions = false;
	}

	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);                                         // lo userdata

	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");    // lo userdata, registry table
	assert(lua_istable(l, -1));

	lua_pushlightuserdata(l, lo->GetObject());                  // lo userdata, registry table, o lightuserdata
	lua_pushvalue(l, -3);                                       // lo userdata, registry table, o lightuserdata, lo userdata
	lua_settable(l, -3);                                        // lo userdata, registry table

	lua_pop(l, 1);                                              // lo userdata

	luaL_getmetatable(l, lo->m_type);                           // lo userdata, lo metatable
	lua_setmetatable(l, -2);                                    // lo userdata

	// attach properties table if available
	PropertiedObject *po = dynamic_cast<PropertiedObject*>(lo->GetObject());
	if (po) {
		po->Properties().PushLuaTable();
		lua_setuservalue(l, -2);
	}

	LUA_DEBUG_END(l, 0);
}

void LuaObjectBase::Deregister(LuaObjectBase *lo)
{
	LuaWrappable *o = lo->GetObject();

	lua_State *l = Lua::manager->GetLuaState();

	LUA_DEBUG_START(l);

	lua_getfield(l, LUA_REGISTRYINDEX, "LuaObjectRegistry");
	assert(lua_istable(l, -1));

	lua_pushlightuserdata(l, o);
	lua_pushnil(l);
	lua_rawset(l, -3);

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);

	return;
}

LuaWrappable *LuaObjectBase::CheckFromLua(int index, const char *type)
{
	assert(instantiated);

	lua_State *l = Lua::manager->GetLuaState();

	luaL_checktype(l, index, LUA_TUSERDATA);
	LuaObjectBase *lo = static_cast<LuaObjectBase*>(lua_touserdata(l, index));

	LuaWrappable *o = lo->GetObject();
	if (!o) {
		luaL_error(l, "Object is no longer valid");
		return 0;
	}

	if (!lo->Isa(type))
		luaL_error(l, "Object on stack has type %s which can not be used as type %s\n", lo->m_type, type);

	// found it
	return o;
}

LuaWrappable *LuaObjectBase::GetFromLua(int index, const char *type)
{
	assert(instantiated);

	lua_State *l = Lua::manager->GetLuaState();

	if (lua_type(l, index) != LUA_TUSERDATA)
		return 0;

	LuaObjectBase *lo = static_cast<LuaObjectBase*>(lua_touserdata(l, index));

	LuaWrappable *o = lo->GetObject();
	if (!o)
		return 0;

	if (!lo->Isa(type))
		return 0;

	// found it
	return o;
}

bool LuaObjectBase::Isa(const char *base) const
{
	// fast path
	if (strcmp(m_type, base) == 0)
		return true;

	assert(instantiated);

	lua_State *l = Lua::manager->GetLuaState();

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

void *LuaObjectBase::Allocate(size_t n) {
	lua_State *l = Lua::manager->GetLuaState();
	return lua_newuserdata(l, n);
}
