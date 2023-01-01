// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "utils.h"
#include <lua.hpp>
#include <stdexcept>
#include <string>
#include <vector>

template <typename FlagType>
struct LuaFlags {
	std::vector<std::pair<const char *, FlagType>> LUT;
	std::string typeName;
	int lookupTableRef = LUA_NOREF;

	LuaFlags(std::initializer_list<std::pair<const char *, FlagType>> init) :
		LUT(init) {}

	void Register(lua_State *l, std::string name)
	{
		if (lookupTableRef != LUA_NOREF)
			throw std::runtime_error(std::string("Class ") + name + " is already registered.");

		lua_newtable(l);
		typeName = name;

		for (auto &pair : LUT) {
			lua_pushstring(l, pair.first);
			lua_pushinteger(l, static_cast<unsigned int>(pair.second));
			lua_settable(l, -3);
		}

		// luaL_ref pops the table off the stack.
		lookupTableRef = luaL_ref(l, LUA_REGISTRYINDEX);
	}

	void Unregister(lua_State *l)
	{
		luaL_unref(l, LUA_REGISTRYINDEX, lookupTableRef);
		lookupTableRef = LUA_NOREF;
	}

	FlagType LookupTable(lua_State *l, int index)
	{
		FlagType flagAccum = FlagType(0);
		index = lua_absindex(l, index);
		if (!lua_istable(l, index) || lookupTableRef == LUA_NOREF) return flagAccum;

		lua_checkstack(l, 2);
		lua_pushinteger(l, lookupTableRef);
		lua_gettable(l, LUA_REGISTRYINDEX);

		int table_idx = 1;
		while (true) {
			lua_pushinteger(l, table_idx++);
			lua_gettable(l, index);

			if (!lua_isstring(l, -1)) {
				lua_pop(l, 1);
				break;
			}

			flagAccum = static_cast<FlagType>(flagAccum | checkFlag(l, -1, -2));
		}

		lua_pop(l, 1);
		return flagAccum;
	}

	FlagType LookupEnum(lua_State *l, int index)
	{
		FlagType flagAccum = FlagType(0);
		index = lua_absindex(l, index);
		if (!lua_isstring(l, index) || lookupTableRef == LUA_NOREF) return flagAccum;

		lua_checkstack(l, 2);
		lua_pushinteger(l, lookupTableRef);
		lua_gettable(l, LUA_REGISTRYINDEX);

		return checkFlag(l, index, -1);
	}

private:
	inline FlagType checkFlag(lua_State *l, int index, int lookup_index)
	{
		lookup_index = lua_absindex(l, lookup_index);
		lua_pushvalue(l, index);
		lua_gettable(l, lookup_index);
		if (lua_isnumber(l, -1)) {
			// bitwise operations implicitly convert to int, so we must explicitly convert back to FlagType.
			FlagType fl_ret = static_cast<FlagType>(lua_tointeger(l, -1));
			lua_pop(l, 2); // clean up the stack!
			return fl_ret;
		} else {
			lua_pop(l, 1);
			std::string index_name = lua_tostring(l, index);
			lua_pop(l, 2); // clean up the stack!
			luaL_error(l, "Unknown %s %s", typeName.c_str(), index_name.c_str());
		}
		return FlagType(0);
	}
};
