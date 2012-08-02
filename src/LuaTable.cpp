#include "LuaTable.h"

lua_State * PersistentTable::g_lua = 0;
static int id_count = 0;
std::vector<int> PersistentTable::g_copy_count(1, 0);

void PersistentTable::Init(lua_State * l) {
	assert(g_lua == 0); // Only one Lua stack is supported
	g_lua = l;
}

void PersistentTable::Uninit(lua_State * l) {
	assert(g_lua == l);
	g_lua = 0;
}


PersistentTable::PersistentTable(const PersistentTable & ref): LuaTable(static_cast<const LuaTable &>(ref)), m_id(ref.m_id) {
	g_copy_count[m_id]++;
}

const PersistentTable & PersistentTable::operator=(const PersistentTable & ref) {
	static_cast<LuaTable *>(this)->operator=(static_cast<const LuaTable &>(ref));

	if (m_id != 0 && g_lua != 0 && m_lua == g_lua) {
		g_copy_count[m_id]--;
		CheckCopyCount();
	}
	m_id = ref.m_id;
	if(m_lua && m_id)
		g_copy_count[m_id]++;
	return *this;
}

PersistentTable::~PersistentTable() {
	if (m_id == 0 || g_lua == 0 || m_lua != g_lua)
		return;
	g_copy_count[m_id]--;
	CheckCopyCount();
}

bool PersistentTable::operator==(const PersistentTable & ref) const {
	if (ref.m_lua != m_lua)
		return false;
	if (ref.m_id == m_id)
		return true;

	assert(m_lua == g_lua);
	ref.PushCopyToStack();
	PushCopyToStack();
	bool return_value = lua_compare(m_lua, -1, -2, LUA_OPEQ);
	lua_pop(m_lua, 2);
	return return_value;
}

void PersistentTable::CheckCopyCount() {
	if (g_copy_count[m_id] <= 0) {
		PushGlobalToStack();
		lua_pushinteger(g_lua, m_id);
		lua_pushnil(g_lua);
		lua_settable(g_lua, -3);
		lua_pop(g_lua, 1);
	}
}

PersistentTable::PersistentTable(lua_State * l, int index): LuaTable(l, index), m_id(++id_count) {
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


void PersistentTable::PushCopyToStack() const {
	assert(g_lua && m_lua == g_lua);
	PushGlobalToStack();
	lua_pushinteger(m_lua, m_id);
	lua_gettable(m_lua, -2);
	lua_remove(m_lua, -2);
}

void PersistentTable::PushGlobalToStack() const {
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

