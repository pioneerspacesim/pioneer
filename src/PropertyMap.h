// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PROPERTYMAP_H
#define PROPERTYMAP_H

#include "LuaManager.h"
#include "LuaRef.h"
#include "LuaTable.h"
#include "libs.h"

struct lua_State;

class PropertyMap {
public:
	PropertyMap(LuaManager *lua);

	template <class Value> void Set(const std::string &k, const Value &v) {
		ScopedTable(m_table).Set(k, v);
		SendSignal(k);
	}

	template <class Value> void Get(const std::string &k, Value &v) {
		v = ScopedTable(m_table).Get<Value>(k, v);
	}

	void PushLuaTable();

	sigc::connection Connect(const std::string &k, const sigc::slot<void,PropertyMap &,const std::string &> &fn) {
		return m_signals[k].connect(fn);
	}

private:
	LuaRef m_table;

	void SendSignal(const std::string &k);
	std::map< std::string,sigc::signal<void,PropertyMap &,const std::string &> > m_signals;
};

#endif
