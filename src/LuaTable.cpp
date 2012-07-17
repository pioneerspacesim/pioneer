#include "LuaTable.h"

lua_State * LuaTable::g_lua = 0;
static int id_count = 0;
std::vector<int> LuaTable::g_copy_count(1, 0);

void LuaTable::Init(lua_State * l) {
	assert(g_lua == 0); // Only one Lua stack is supported
	g_lua = l;
}

void LuaTable::Uninit(lua_State * l) {
	assert(g_lua == l);
	g_lua = 0;
}

LuaTable::LuaTable(): m_id(0), m_lua(0) {}

LuaTable::LuaTable(const LuaTable & ref): m_id(ref.m_id), m_lua(ref.m_lua) {
	g_copy_count[m_id]++;
}

const LuaTable & LuaTable::operator=(const LuaTable & ref) {
	if (m_id != 0 && g_lua != 0 && m_lua == g_lua) {
		g_copy_count[m_id]--;
		CheckCopyCount();
	}
	m_id = ref.m_id;
	m_lua = ref.m_lua; 
	if(m_lua && m_id)
		g_copy_count[m_id]++;
	return *this;
}

LuaTable::~LuaTable() {
	if (m_id == 0 || g_lua == 0 || m_lua != g_lua)
		return;
	g_copy_count[m_id]--;
	CheckCopyCount();
}

void LuaTable::CheckCopyCount() {
	if (g_copy_count[m_id] <= 0) {
		PushGlobalToStack();
		lua_pushinteger(g_lua, m_id);
		lua_pushnil(g_lua);
		lua_settable(g_lua, -3);
		lua_pop(g_lua, 1);
	}
}

LuaTable::LuaTable(lua_State * l, int index): m_id(++id_count), m_lua(l) {
	assert(g_lua && m_lua == g_lua);
	if (index != 0)
		index = lua_absindex(m_lua, index);

	PushGlobalToStack();
	lua_pushinteger(m_lua, m_id);
	if (index == 0)
		lua_newtable(m_lua);
	else
		lua_pushvalue(m_lua, index);
	lua_settable(m_lua, -3);
	lua_pop(m_lua, 1);

	g_copy_count.push_back(1);
}

void LuaTable::PushCopyToStack() const {
	assert(g_lua && m_lua == g_lua);
	PushGlobalToStack();
	lua_pushinteger(m_lua, m_id);
	lua_gettable(m_lua, -2);
	lua_remove(m_lua, -2);
}

void LuaTable::PushGlobalToStack() const {
	lua_pushstring(m_lua, "LuaTable");
	lua_gettable(m_lua, LUA_REGISTRYINDEX);
	if (lua_isnil(m_lua, -1)) {
		lua_pop(m_lua, 1);
		lua_newtable(m_lua);
		lua_pushstring(m_lua, "LuaTable");
		lua_pushvalue(m_lua, -2);
		lua_settable(m_lua, LUA_REGISTRYINDEX);
	}
}

