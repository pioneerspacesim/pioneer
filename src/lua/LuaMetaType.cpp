// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaMetaType.h"
#include "Lua.h"
#include "LuaObject.h"
#include "LuaPropertyMap.h"
#include "core/Property.h"

#include "utils.h"

// if found, returns true, leaves item to return to lua on top of stack
// if not found, returns false
static bool get_method(lua_State *l, int metatable, int name)
{
	LUA_DEBUG_START(l);

	name = lua_absindex(l, name);

	// look up the name in the list of methods.
	lua_getfield(l, metatable, "methods");
	lua_pushvalue(l, name); // make a copy of the name
	lua_rawget(l, -2);		// look it up in the methods table
	lua_remove(l, -2);		// remove the methods table

	// found something, return it
	if (!lua_isnil(l, -1)) {
		LUA_DEBUG_END(l, 1);
		return true;
	}
	lua_pop(l, 1);

	// otherwise, just return
	LUA_DEBUG_END(l, 0);
	return false;
}

// if found, returns true, leaves attribute entry on top of stack
// if not found, returns false
static bool get_attr_entry(lua_State *l, int metatable, int name)
{
	LUA_DEBUG_START(l);

	name = lua_absindex(l, name);

	// lookup the name in the list of attributes
	lua_getfield(l, metatable, "attrs");
	lua_pushvalue(l, name); // make a copy of the name
	lua_rawget(l, -2);		// look it up in the methods table
	lua_remove(l, -2);		// remove the attributes table

	// found an attribute entry, don't evaluate
	if (!lua_isnil(l, -1)) {
		LUA_DEBUG_END(l, 1);
		return true;
	}
	lua_pop(l, 1);

	// not found
	LUA_DEBUG_END(l, 0);
	return false;
}

// takes the metatable, name on top of stack
// Look up a method or attribute, evaluating the attribute entry as a getter.
static bool get_method_or_attr(lua_State *l)
{
	LUA_DEBUG_START(l);

	// if we have a method, call it
	if (get_method(l, -2, -1)) {
		LUA_DEBUG_END(l, 1);
		return true;
	} else if (get_attr_entry(l, -2, -1)) {
		// Someone may have put a non-function value in the attributes
		// weird, but ok, we can handle it
		if (lua_isfunction(l, -1)) {
			lua_pushvalue(l, 1);			// push the self object
			pi_lua_protected_call(l, 1, 1); // call the attribute entry

			LUA_DEBUG_END(l, 1);
			return true;
		}

		LUA_DEBUG_END(l, 1);
		return true;
	}

	LUA_DEBUG_END(l, 0);
	return false;
}

static int l_index(lua_State *l)
{
	// userdata are typed, tables are not
	bool typeless = lua_istable(l, 1);
	assert(typeless || lua_isuserdata(l, 1));

	// typeless objects have no parents, and have only one method table, so this is easy
	if (typeless) {
		lua_getmetatable(l, 1);
		lua_pushvalue(l, 2);

		// if we have something in the first metatable, return it
		if (get_method_or_attr(l))
			return 1;

		// otherwise, nothing further to look up
		return 0;
	}

	// normal userdata object
	// first check properties. we don't need to drill through lua if the
	// property is already available
	if (lua_isstring(l, 2)) {
		PropertyMap *map = LuaObjectBase::GetPropertiesFromObject(l, 1);
		if (map) {
			auto &prop = map->Get(LuaPull<std::string_view>(l, 2));
			if (!prop.is_null()) {
				LuaPush(l, prop);
				return 1;
			}
		}
	}

	// push the metatype registry here for later
	lua_getfield(l, LUA_REGISTRYINDEX, "LuaMetaTypes");
	int metaTypeRegistry = lua_gettop(l);

	// push metatable, name onto the top of the stack
	lua_getmetatable(l, 1);
	lua_pushvalue(l, 2);
	while (1) {
		// got a method or an attribute handler from this metatype
		if (get_method_or_attr(l)) {
			// clean up the stack
			lua_insert(l, -3);
			lua_pop(l, 2);
			return 1;
		}

		// if there's no parent metatable, get out
		lua_getfield(l, -2, "parent");
		if (lua_isnil(l, -1))
			break;

		std::string parentName = lua_tostring(l, -1);
		lua_pop(l, 1); // pop the parent name

		lua_getfield(l, metaTypeRegistry, parentName.c_str());
		if (lua_isnil(l, -1))
			return luaL_error(l, "Encountered invalid parent metatype name %s", parentName.c_str());

		lua_replace(l, -3); // replace the metatable with the parent
	}

	return 0;
}

static int l_newindex(lua_State *l)
{
	// userdata are typed, tables are not
	bool typeless = lua_istable(l, 1);
	assert(typeless || lua_isuserdata(l, 1));

	// Attribute setters are not enabled for typeless objects.
	if (typeless) {
		lua_rawset(l, 1); // set the value in the table
		return 0;
	}

	// Once we've dealt with the chance of a typeless object, the only thing
	// left is userdata.

	// First, check properties. We don't need to drill through the metatype stack
	// if the property is already available.
	// Properties take precedence over attrs only if they've been previously set
	// (use setprop if you want to be sure you're setting a property)
	if (lua_isstring(l, 2)) {
		PropertyMap *map = LuaObjectBase::GetPropertiesFromObject(l, 1);
		if (map) {
			auto key = LuaPull<std::string_view>(l, 2);

			if (!map->Get(key).is_null()) {
				map->Set(key, LuaPull<Property>(l, 3));
				return 0;
			}
		}
	}

	// push the metatype registry here for later
	lua_getfield(l, LUA_REGISTRYINDEX, "LuaMetaTypes");
	int metaTypeRegistry = lua_gettop(l);

	// Check the metatable for attributes
	lua_getmetatable(l, 1); // get the metatable
	while (true) {
		// if we have an attribute handler, call it
		// if the attribute entry is a non-function value, it is considered immutable
		if (get_attr_entry(l, -1, 2)) {
			if (lua_isfunction(l, -1)) {
				lua_remove(l, -2);	 // remove the metatable
				lua_pushvalue(l, 1); // push the self object
				lua_pushvalue(l, 3); // push the value
				pi_lua_protected_call(l, 2, 0);
				return 0;
			}
			lua_pop(l, 1);
		}

		lua_getfield(l, -1, "parent"); // get the parent field from the metatable
		if (lua_isnil(l, -1))		   // hit the end of the chain, nothing here
			break;

		std::string parentName = lua_tostring(l, -1);
		lua_pop(l, 1);

		lua_getfield(l, metaTypeRegistry, parentName.c_str());
		if (lua_isnil(l, -1))
			return luaL_error(l, "Encountered invalid parent metatype name %s", parentName.c_str());

		lua_remove(l, -2); // replace the metatable with the parent
	}

	return luaL_error(l, "Attempt to set undefined property %s on %s", lua_tostring(l, 2), lua_tostring(l, 1));
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

		size_t str_size = 0;
		const char *name = lua_tolstring(l, -2, &str_size);
		// anything starting with an underscore is hidden
		if (name[0] == '_') {
			lua_pop(l, 1);
			continue;
		}

		// check if the name begins with the prefix
		if (strncmp(name, prefix.c_str(), std::min(str_size, prefix.size())) == 0)
			// push back the portion of the name not matching the prefix (for completion)
			names.push_back(std::string(name + prefix.size()));

		lua_pop(l, 1);
	}
}

void LuaMetaTypeBase::GetNames(std::vector<std::string> &names, const std::string &prefix, bool methodsOnly)
{
	// never show hidden names
	if (!prefix.empty() && prefix[0] == '_')
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

	else {
		// it's a non-table object with a metatable
		// this realistically can only be a string, but let's work with it anyways
		lua_getmetatable(l, -1);
		if (lua_isnil(l, -1)) {
			lua_pop(l, 1);
			return; // if no metatable, can't do anything
		}

		lua_getfield(l, -1, "__index");
		typeless = lua_istable(l, -1);
		lua_pop(l, 2);

		// if the __index field isn't a table, we can't introspect into it
		if (!typeless)
			return;
	}

	LUA_DEBUG_CHECK(l, 0);

	if (typeless) {
		// if the object we're getting names from is a table, search it for names
		if (lua_istable(l, -1))
			get_names_from_table(l, names, prefix, methodsOnly);

		// Check the metatable indexes
		// Get the metatable of the object, then check it for an __index field
		lua_pushvalue(l, -1);
		while (lua_getmetatable(l, -1)) {
			lua_getfield(l, -2, "__index");

			// Replace the metatable with the index table to keep a stable stack size.
			lua_replace(l, -3);
			lua_pop(l, 1);

			if (lua_istable(l, -1))
				get_names_from_table(l, names, prefix, methodsOnly);
			else
				break;
		}
		lua_pop(l, 1);

		return;
	}

	// properties
	if (!methodsOnly) {
		// TODO: iterate over PropertyMap if we need this functionality
		// lua_getuservalue(l, -1);
		// if (!lua_istable(l, -1))
		// 	get_names_from_table(l, names, prefix, false);
		// lua_pop(l, 1);
	}

	// check the metatable (and its parents) for methods and attributes
	lua_getmetatable(l, -1);
	while (1) {
		lua_getfield(l, -1, "methods");
		get_names_from_table(l, names, prefix, methodsOnly);
		lua_pop(l, 1);

		if (!methodsOnly) {
			lua_getfield(l, -1, "attrs");
			get_names_from_table(l, names, prefix, false);
			lua_pop(l, 1);
		}

		lua_getfield(l, -1, "parent");
		std::string parent = lua_tostring(l, -1);
		if (!lua_isnil(l, -1))
			parent = lua_tostring(l, -1);

		lua_pop(l, 1); // pop the parent's name

		bool hasParent = !parent.empty() && GetMetatableFromName(l, parent.c_str());
		if (hasParent)
			lua_remove(l, -2); // replace the previous metatype with the parent
		else
			break; // the old metatype is the only thing on the stack
	}

	lua_pop(l, 1); // pop the metatable

	LUA_DEBUG_END(l, 0);
}

bool LuaMetaTypeBase::GetMetatableFromName(lua_State *l, const char *name)
{
	luaL_getsubtable(l, LUA_REGISTRYINDEX, "LuaMetaTypes");
	lua_getfield(l, -1, name);
	lua_remove(l, -2);
	if (lua_isnil(l, -1)) {
		lua_pop(l, 1);
		return false;
	}

	return true;
}

void LuaMetaTypeBase::GetMetatable() const
{
	assert(IsValid());
	LUA_DEBUG_START(m_lua);

	luaL_getsubtable(m_lua, LUA_REGISTRYINDEX, "LuaMetaTypes");
	lua_rawgeti(m_lua, -1, m_ref);
	lua_remove(m_lua, -2);
	assert(lua_type(m_lua, -1) == LUA_TTABLE);

	LUA_DEBUG_END(m_lua, 1);
}

void *LuaMetaTypeBase::TestUserdata(lua_State *l, int index, const char *type)
{
	void *p = lua_touserdata(l, index);
	if (p != nullptr && lua_getmetatable(l, index)) {
		if (GetMetatableFromName(l, type) && lua_rawequal(l, -1, -2)) {
			lua_pop(l, 2);
			return p;
		}
		lua_pop(l, 1);
	}
	return nullptr;
}

void *LuaMetaTypeBase::CheckUserdata(lua_State *l, int index, const char *type)
{
	void *p = TestUserdata(l, index, type);
	if (!p) {
		const char *msg = lua_pushfstring(l, "%s expected, got %s", type, luaL_typename(l, index));
		luaL_argerror(l, index, msg);
	}

	return p;
}

void LuaMetaTypeBase::CreateMetaType(lua_State *l)
{
	luaL_getsubtable(l, LUA_REGISTRYINDEX, "LuaMetaTypes");
	m_lua = l;

	// if the type name is empty, we're creating a "throwaway" object with no parent either
	if (!m_typeName.empty()) {
		// Warn if we're double-initializing a type name
		lua_getfield(l, -1, m_typeName.c_str());
		if (!lua_isnil(l, -1))
			Output("Double-initialization of lua metatype %s. Do you have a name conflict?\n", m_typeName.c_str());
		lua_pop(l, 1);
	}

	lua_newtable(l);

	if (!m_typeName.empty()) {
		// Set the entry LuaMetaTypes[typename] = metatype
		lua_pushvalue(l, -1);
		lua_setfield(l, -3, m_typeName.c_str());
	}

	// Store this metatable via a numeric ref index for easy access
	lua_pushvalue(l, -1);
	m_ref = luaL_ref(l, -3);

	// set the type name on the metatable
	lua_pushstring(l, m_typeName.c_str());
	lua_setfield(l, -2, "type");

	if (!m_parent.empty()) {
		lua_pushstring(l, m_parent.c_str());
		lua_setfield(l, -2, "parent");
	}

	// create the attributes table
	lua_newtable(l);
	lua_setfield(l, -2, "attrs");

	// create the methods table
	lua_newtable(l);
	// create the methods metatable
	lua_newtable(l);
	lua_setmetatable(l, -2);
	lua_setfield(l, -2, "methods");

	lua_pushcclosure(l, &l_index, 0);
	lua_setfield(l, -2, "__index");

	lua_pushcclosure(l, &l_newindex, 0);
	lua_setfield(l, -2, "__newindex");

	// replace the LuaMetaTypes registry table, leaving the created metatype on the stack
	lua_replace(l, -2);
}
