// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PROPERTYMAP_H
#define PROPERTYMAP_H

#include "LuaManager.h"
#include "LuaRef.h"
#include <string>

struct lua_State;

class PropertyMap {
public:
	PropertyMap(LuaManager *lua);

	void Set(const std::string &k, bool v);
	void Set(const std::string &k, int v);
	void Set(const std::string &k, double v);
	void Set(const std::string &k, const std::string &v);
	void Set(const std::string &k, const char *v) { Set(k, std::string(v)); }

	void Get(const std::string &k, bool &v);
	void Get(const std::string &k, int &v);
	void Get(const std::string &k, double &v);
	void Get(const std::string &k, std::string &v);

	void PushLuaTable();

private:
	LuaRef m_table;
};

#endif
