// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PropertyHolder.h"
#include "LuaUtils.h"

#if 0
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
#endif


PropertyHolder::PropertyHolder(LuaManager *lua)
{
	lua_State *l = lua->GetLuaState();
	lua_newtable(l);
	m_table = LuaRef(l, -1);
}


void PropertyHolder::SetProperty(const std::string &k, bool v)
{
	lua_State *l = m_table.GetLua();
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_pushboolean(l, v);
	lua_rawset(l, -3);
	lua_pop(l, 1);
}

void PropertyHolder::SetProperty(const std::string &k, int v)
{
	lua_State *l = m_table.GetLua();
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_pushinteger(l, v);
	lua_rawset(l, -3);
	lua_pop(l, 1);
}

void PropertyHolder::SetProperty(const std::string &k, double v)
{
	lua_State *l = m_table.GetLua();
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_pushnumber(l, v);
	lua_rawset(l, -3);
	lua_pop(l, 1);
}

void PropertyHolder::SetProperty(const std::string &k, const std::string &v)
{
	lua_State *l = m_table.GetLua();
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_pushlstring(l, v.c_str(), v.size());
	lua_rawset(l, -3);
	lua_pop(l, 1);
}


void PropertyHolder::GetProperty(const std::string &k, bool &v)
{
	lua_State *l = m_table.GetLua();
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_rawget(l, -2);
	v = lua_toboolean(l, -1);
	lua_pop(l, 2);
}

void PropertyHolder::GetProperty(const std::string &k, int &v)
{
	lua_State *l = m_table.GetLua();
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_rawget(l, -2);
	v = lua_tointeger(l, -1);
	lua_pop(l, 2);
}

void PropertyHolder::GetProperty(const std::string &k, double &v)
{
	lua_State *l = m_table.GetLua();
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_rawget(l, -2);
	v = lua_tonumber(l, -1);
	lua_pop(l, 2);
}

void PropertyHolder::GetProperty(const std::string &k, std::string &v)
{
	lua_State *l = m_table.GetLua();
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_rawget(l, -2);
	v = lua_tostring(l, -1);
	lua_pop(l, 2);
}
