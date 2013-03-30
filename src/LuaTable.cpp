#include "LuaTable.h"

template <> void LuaTable::VecIter<LuaTable>::CleanCache() {
	if (m_cache) {
		lua_remove(m_table->GetLua(), m_cache->GetIndex());
		delete m_cache;
		m_cache = 0;
	}
}
template <> LuaTable * LuaTable::VecIter<LuaTable>::GetCache() {
	if (m_cache == 0) {
		m_cache = new LuaTable;
		*m_cache = m_table->Sub(m_currentIndex);
	}
	return m_cache;
}

