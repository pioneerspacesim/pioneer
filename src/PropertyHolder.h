// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PROPERTYHOLDER_H
#define PROPERTYHOLDER_H

#include "LuaManager.h"
#include "LuaRef.h"
#include <string>

struct lua_State;

class PropertyHolder {
public:
	void SetProperty(const std::string &k, bool v);
	void SetProperty(const std::string &k, int v);
	void SetProperty(const std::string &k, double v);
	void SetProperty(const std::string &k, const std::string &v);

	void GetProperty(const std::string &k, bool &v);
	void GetProperty(const std::string &k, int &v);
	void GetProperty(const std::string &k, double &v);
	void GetProperty(const std::string &k, std::string &v);

	/*
	bool PushPropertyToLua(lua_State *l, const std::string &k);
	void AddPropertiesToLuaTable(lua_State *l, int tableIdx);
	*/

protected:
	PropertyHolder(LuaManager *lua);

private:
	LuaRef m_table;
};

#endif
