// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CoreFwdDecl.h"
#include "FileSystem.h"
#include "core/Log.h"
#include "utils.h"
#include "lua.h"

// Forward declarations
std::string pi_lua_tostring(lua_State *l, int idx, int indent = 0, int recurseTable = -1);
std::string pi_lua_traceback(lua_State *l, int top);

// ==================================================

static std::string_view _get_funcsource(lua_Debug *entry)
{
	std::string_view src = entry->source;
	if (starts_with(src, "@[T] ")) {
		return src.substr(5); // strip trusted mark from filename
	} else if (starts_with(src, "@")) {
		return src.substr(1); // strip mark from filename
	} else {
		return entry->short_src; // let lua figure it out
	}
}

static std::string _get_funcname(lua_State *l, lua_Debug *entry)
{
	if (entry->namewhat[0] != '\0') {
		return fmt::format("function {}", entry->name);
	} else if (entry->what[0] == 'm') {
		return "main chunk";
	} else if (entry->what[0] == 'C') {
		return "?";
	} else {
		return fmt::format("function <{}:{}>", _get_funcsource(entry), entry->linedefined);
	}
}

static std::string _iterate_locals(lua_State *l, lua_Debug *entry, int recurseTable, int startIdx, int endIdx = 0)
{
	// Iterate parameters or locals
	std::string accum = "";
	int stackIdx = lua_gettop(l) + 1;
	const char *paramName = nullptr;

	while ((paramName = lua_getlocal(l, entry, startIdx)) != nullptr) {
		if (!starts_with(paramName, "(*temp")) // filter out lua temporaries
			accum += fmt::format("\t\t{} = {}\n", paramName, pi_lua_tostring(l, stackIdx, 2, recurseTable));
		lua_pop(l, 1);

		startIdx += (startIdx < 0 ? -1 : 1);
		if (endIdx && startIdx > endIdx)
			break;
	}

	return accum;
}

std::string pi_lua_traceback(lua_State *l, int top)
{
	LUA_DEBUG_START(l);
	std::string accum = "stack traceback:\n";

	lua_Debug entry;
	int depth = top;

	while (lua_getstack(l, depth, &entry)) {
		lua_getinfo(l, "Slntu", &entry);

		accum += fmt::format("\t{}:{} in {}\n", _get_funcsource(&entry), entry.currentline, _get_funcname(l, &entry));

		if (entry.istailcall)
			accum += "\t(...tail calls...)\n";

		depth++;
	}

	LUA_DEBUG_END(l, 0);
	return accum;
}

std::string pi_lua_dumpstack(lua_State *l, int top)
{
	LUA_DEBUG_START(l);
	std::string accum = "stack dump:\n";

	lua_newtable(l);
	int recurseTable = lua_gettop(l);

	lua_Debug entry;
	int depth = top;

	while (lua_getstack(l, depth, &entry)) {
		lua_getinfo(l, "Slntu", &entry);

		accum += fmt::format("\t#{}: {}:{}", depth - top + 1, _get_funcsource(&entry), entry.currentline);

		accum += fmt::format(" in {}:\n", _get_funcname(l, &entry));

		if (entry.what[0] == 'C') {
			accum += _iterate_locals(l, &entry, recurseTable, -1);
		} else {
			if (entry.nparams)
				accum += _iterate_locals(l, &entry, recurseTable, 1, entry.nparams);
			if (entry.isvararg)
				accum += _iterate_locals(l, &entry, recurseTable, -1);

			accum += _iterate_locals(l, &entry, recurseTable, entry.nparams + 1);
		}

		accum += "\n";

		if (entry.istailcall)
			accum += "\t(...tail calls...)\n";

		depth++;
	}

	lua_pop(l, 1);

	LUA_DEBUG_END(l, 0);
	return accum;
}

// recurse through a table and print its values
std::string _pi_lua_table_tostring(lua_State *l, int idx, int indent, int recurseTable)
{
	LUA_DEBUG_START(l);
	lua_checkstack(l, 2);

	if (recurseTable > 0) {
		lua_pushvalue(l, idx);
		lua_rawget(l, recurseTable);

		if (!lua_isnil(l, -1)) {
			lua_pop(l, 1);
			LUA_DEBUG_END(l, 0);
			return "[R]";
		}

		lua_pop(l, 1);
		lua_pushvalue(l, idx);
		lua_pushboolean(l, true);
		lua_rawset(l, recurseTable);
	}

	std::string accum = "{\n";

	int top = lua_gettop(l);

	// count number of keys in table
	int iter = 0;

	lua_pushnil(l);
	while (lua_next(l, idx) != 0) {
		lua_pop(l, 1);
		++iter;
	}

	// Too many keys in this table, don't pollute the log
	if (iter >= 30) {
		LUA_DEBUG_END(l, 0);
		return "{ ... }";
	}

	lua_pushnil(l);
	while (lua_next(l, idx) != 0) {
		accum.append(indent + 1, '\t');
		accum += fmt::format("{} = {}\n",
			pi_lua_tostring(l, top + 1, indent + 1, recurseTable),
			pi_lua_tostring(l, top + 2, indent + 1, recurseTable));
		lua_pop(l, 1); // pop the value and leave the key on the stack
	}

	accum.append(indent, '\t');
	accum += "}";

	LUA_DEBUG_END(l, 0);
	return accum;
}

// Return a debug representation of the given lua value
std::string pi_lua_tostring(lua_State *l, int idx, int indent, int recurseTable)
{
	lua_checkstack(l, 4);

	int type = lua_type(l, idx);

	switch (type) {
		case LUA_TNIL:
		case LUA_TBOOLEAN:
		case LUA_TNUMBER:
		case LUA_TSTRING:
		case LUA_TUSERDATA: {
			std::string str = luaL_tolstring(l, idx, NULL);
			lua_pop(l, 1);
			return str;
		}

		case LUA_TLIGHTUSERDATA:
			return fmt::format("<ptr {}>", lua_touserdata(l, idx));

		case LUA_TTABLE: {
			if (luaL_getmetafield(l, idx, "class")) {
				if (lua_isstring(l, -1)) {
					std::string className = lua_tostring(l, -1);
					lua_pop(l, 1);
					return fmt::format("<table {} ({})> {}", className, lua_topointer(l, idx),
						_pi_lua_table_tostring(l, idx, indent, recurseTable));
				}
			}

			return fmt::format("<table ({})> {}", lua_topointer(l, idx),
				_pi_lua_table_tostring(l, idx, indent, recurseTable));
		}
		case LUA_TFUNCTION: {
			lua_Debug ar;
			lua_pushvalue(l, idx);
			lua_getinfo(l, ">S", &ar);
			if (ar.what[0] == 'C')
				return fmt::format("<function [C] {}>", lua_topointer(l, idx));
			else
				return fmt::format("<function {}:{}>", _get_funcsource(&ar), ar.linedefined);
		}
		case LUA_TTHREAD: {
			lua_State *thread = lua_tothread(l, idx);
			if (lua_status(thread) == LUA_YIELD)
				return fmt::format("<yielded thread {}> @ {}", lua_topointer(l, idx), pi_lua_traceback(thread, 0));
			else
				return fmt::format("<thread {}>", lua_topointer(l, idx));
		}
		default:
			return "<none>";
	}
}

// Pretty-print a lua value to the console for debugging
void pi_lua_printvalue(lua_State *l, int idx)
{
	Log::Info("{}\n", pi_lua_tostring(l, lua_absindex(l, idx)));
}

// https://zeux.io/2010/11/07/lua-callstack-with-c-debugger/
void pi_lua_stacktrace(lua_State *l)
{
	lua_Debug entry;
	int depth = 0;

	while (lua_getstack(l, depth, &entry)) {
		int status = lua_getinfo(l, "Sln", &entry);
		assert(status);

		Output("%s(%d): %s\n", entry.short_src, entry.currentline, entry.name ? entry.name : "?");
		depth++;
	}
}
