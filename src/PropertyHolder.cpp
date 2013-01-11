// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PropertyHolder.h"
#include "LuaUtils.h"

template <> void TypedPropertyHolder<bool>::AddPropertiesToLuaTable(lua_State *l, int tableIdx)
{
	tableIdx = lua_absindex(l, tableIdx);
	for (PropertyMapIterator i = PropertiesBegin(); i != PropertiesEnd(); ++i) {
		lua_pushlstring(l, i->first.c_str(), i->first.size());
		lua_pushboolean(l, i->second);
		lua_rawset(l, tableIdx);
	}
}

template <> void TypedPropertyHolder<int>::AddPropertiesToLuaTable(lua_State *l, int tableIdx)
{
	tableIdx = lua_absindex(l, tableIdx);
	for (PropertyMapIterator i = PropertiesBegin(); i != PropertiesEnd(); ++i) {
		lua_pushlstring(l, i->first.c_str(), i->first.size());
		lua_pushinteger(l, i->second);
		lua_rawset(l, tableIdx);
	}
}

template <> void TypedPropertyHolder<double>::AddPropertiesToLuaTable(lua_State *l, int tableIdx)
{
	tableIdx = lua_absindex(l, tableIdx);
	for (PropertyMapIterator i = PropertiesBegin(); i != PropertiesEnd(); ++i) {
		lua_pushlstring(l, i->first.c_str(), i->first.size());
		lua_pushnumber(l, i->second);
		lua_rawset(l, tableIdx);
	}
}

template <> void TypedPropertyHolder<std::string>::AddPropertiesToLuaTable(lua_State *l, int tableIdx)
{
	tableIdx = lua_absindex(l, tableIdx);
	for (PropertyMapIterator i = PropertiesBegin(); i != PropertiesEnd(); ++i) {
		lua_pushlstring(l, i->first.c_str(), i->first.size());
		lua_pushlstring(l, i->second.c_str(), i->second.size());
		lua_rawset(l, tableIdx);
	}
}

void PropertyHolder::AddPropertiesToLuaTable(lua_State *l, int tableIdx)
{
	TypedPropertyHolder<bool>::AddPropertiesToLuaTable(l, tableIdx);
	TypedPropertyHolder<int>::AddPropertiesToLuaTable(l, tableIdx);
	TypedPropertyHolder<double>::AddPropertiesToLuaTable(l, tableIdx);
	TypedPropertyHolder<std::string>::AddPropertiesToLuaTable(l, tableIdx);
}

template <> bool TypedPropertyHolder<bool>::PushPropertyToLua(lua_State *l, const std::string &k)
{
	typename PropertyMap::iterator i;
	if (!GetIterator(k, i))
		return false;
	lua_pushboolean(l, i->second);
	return true;
}

template <> bool TypedPropertyHolder<int>::PushPropertyToLua(lua_State *l, const std::string &k)
{
	typename PropertyMap::iterator i;
	if (!GetIterator(k, i))
		return false;
	lua_pushinteger(l, i->second);
	return true;
}

template <> bool TypedPropertyHolder<double>::PushPropertyToLua(lua_State *l, const std::string &k)
{
	typename PropertyMap::iterator i;
	if (!GetIterator(k, i))
		return false;
	lua_pushnumber(l, i->second);
	return true;
}

template <> bool TypedPropertyHolder<std::string>::PushPropertyToLua(lua_State *l, const std::string &k)
{
	typename PropertyMap::iterator i;
	if (!GetIterator(k, i))
		return false;
	lua_pushlstring(l, i->second.c_str(), i->second.size());
	return true;
}

bool PropertyHolder::PushPropertyToLua(lua_State *l, const std::string &k)
{
	return
		TypedPropertyHolder<bool>::PushPropertyToLua(l, k) ||
		TypedPropertyHolder<int>::PushPropertyToLua(l, k) ||
		TypedPropertyHolder<double>::PushPropertyToLua(l, k) ||
		TypedPropertyHolder<std::string>::PushPropertyToLua(l, k);
}

