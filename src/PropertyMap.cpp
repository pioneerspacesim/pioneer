// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PropertyMap.h"
#include "LuaUtils.h"

PropertyMap::PropertyMap(LuaManager *lua)
{
	lua_State *l = lua->GetLuaState();
	LUA_DEBUG_START(l);
	lua_newtable(l);
	m_table = LuaRef(l, -1);
	lua_pop(l, 1);
	LUA_DEBUG_END(l, 0);
}


void PropertyMap::Set(const std::string &k, bool v)
{
	lua_State *l = m_table.GetLua();
	LUA_DEBUG_START(l);
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_pushboolean(l, v);
	lua_rawset(l, -3);
	lua_pop(l, 1);
	LUA_DEBUG_END(l, 0);
	SendSignal(k);
}

void PropertyMap::Set(const std::string &k, int v)
{
	lua_State *l = m_table.GetLua();
	LUA_DEBUG_START(l);
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_pushinteger(l, v);
	lua_rawset(l, -3);
	lua_pop(l, 1);
	LUA_DEBUG_END(l, 0);
	SendSignal(k);
}

void PropertyMap::Set(const std::string &k, double v)
{
	lua_State *l = m_table.GetLua();
	LUA_DEBUG_START(l);
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_pushnumber(l, v);
	lua_rawset(l, -3);
	lua_pop(l, 1);
	LUA_DEBUG_END(l, 0);
	SendSignal(k);
}

void PropertyMap::Set(const std::string &k, const std::string &v)
{
	lua_State *l = m_table.GetLua();
	LUA_DEBUG_START(l);
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_pushlstring(l, v.c_str(), v.size());
	lua_rawset(l, -3);
	lua_pop(l, 1);
	LUA_DEBUG_END(l, 0);
	SendSignal(k);
}

void PropertyMap::SendSignal(const std::string &k)
{
	std::map< std::string,sigc::signal<void,PropertyMap &,const std::string &> >::iterator i = m_signals.find(k);
	if (i == m_signals.end())
		return;

	(*i).second.emit(*this, k);
}


void PropertyMap::Get(const std::string &k, bool &v)
{
	lua_State *l = m_table.GetLua();
	LUA_DEBUG_START(l);
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_rawget(l, -2);
	v = lua_toboolean(l, -1);
	lua_pop(l, 2);
	LUA_DEBUG_END(l, 0);
}

void PropertyMap::Get(const std::string &k, int &v)
{
	lua_State *l = m_table.GetLua();
	LUA_DEBUG_START(l);
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_rawget(l, -2);
	v = lua_tointeger(l, -1);
	lua_pop(l, 2);
	LUA_DEBUG_END(l, 0);
}

void PropertyMap::Get(const std::string &k, double &v)
{
	lua_State *l = m_table.GetLua();
	LUA_DEBUG_START(l);
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_rawget(l, -2);
	v = lua_tonumber(l, -1);
	lua_pop(l, 2);
	LUA_DEBUG_END(l, 0);
}

void PropertyMap::Get(const std::string &k, std::string &v)
{
	lua_State *l = m_table.GetLua();
	LUA_DEBUG_START(l);
	m_table.PushCopyToStack();
	lua_pushlstring(l, k.c_str(), k.size());
	lua_rawget(l, -2);
	v = lua_tostring(l, -1);
	lua_pop(l, 2);
	LUA_DEBUG_END(l, 0);
}


void PropertyMap::PushLuaTable()
{
	m_table.PushCopyToStack();
}
