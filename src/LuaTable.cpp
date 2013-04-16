#include "LuaTable.h"

template <> void LuaTable::VecIter<LuaTable>::LoadCache() {
	if (m_dirtyCache) {
		m_cache = m_table->Sub(m_currentIndex);
		m_dirtyCache = false;
	}
}
template <> void LuaTable::VecIter<LuaTable>::CleanCache() {
	if (!m_dirtyCache && m_cache.GetLua()) {
		lua_remove(m_cache.GetLua(), m_cache.GetIndex());
	}
	m_dirtyCache = true;
}

