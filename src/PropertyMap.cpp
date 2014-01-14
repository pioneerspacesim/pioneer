// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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

void PropertyMap::SendSignal(const std::string &k)
{
	std::map< std::string,sigc::signal<void,PropertyMap &,const std::string &> >::iterator i = m_signals.find(k);
	if (i == m_signals.end())
		return;

	(*i).second.emit(*this, k);
}

void PropertyMap::PushLuaTable()
{
	m_table.PushCopyToStack();
}
