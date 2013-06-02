// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUATABLE_H
#define _LUATABLE_H

#include <cassert>
#include <map>
#include <vector>
#include <iterator>

#include "lua/lua.hpp"
#include "LuaRef.h"

/*
 * The LuaTable class is a wrapper around a table present on the stack. There
 * are two ways to instanciate a LuaTable object:
 *
 * > lua_State *l;
 * > int i;
 * > LuaTable(l); // This will allow a new table on top of the stack
 * > LuaTable(l, i); // This will wrap the object around an existing table
 *
 * Note that in no way does destroying a LuaTable object will remove the
 * wrapped table from the stack.
 * Also, there is no integrity check except at the object creation, which means
 * that if you fiddle with the stack below the wrapped table you will get
 * unexpected results (most likely a crash).
 *
 * Get/Set:
 *
 * The Get and Set operators are using the pi_lua_generic_{push pull} functions
 * to fetch any value from the table using templates. It is then possible to
 * add support for extra types only by writing the beforementioned functions
 * for the wanted type. Note that that Get has two flavours, one with a
 * default value and another faster which will crash if the key gives nil.
 *
 * These operations are designed to restore the state of the stack, thus making
 * it impossible to Get a LuaTable, as the latter would need a place on the
 * stack. For this reason, the Sub() method is used to get a subtable,
 * allocating it on top of the stack.
 *
 * Example:
 *
 * > lua_State *l; // stack size: X
 * > LuaTable t = LuaTable(l+1); // stack size: X+1, t = {}
 * > t.Set("foo", 1); // stack size: X+1, t = {foo:1}
 * > int foo = t.Get<int>("foo");
 * > //int bar = t.Get<int>("bar"); // WOULD CRASH!
 * > int bar = t.Get("bar", 0);
 * > {
 * >     LuaTable t2(l); // stack size: X+2
 * >     t.Set("baz", t2); // t = {foo:1, baz:<t2>}
 * > } // t2 isn't a valid name, we can now safely pop the table out.
 * > lua_pop(l, 1); // stack size: X+1
 * > LuaTable t2_bis = t.Sub("baz"); // stack size: X+2
 */
class LuaTable {
public:
	// For now, every lua_State * can only be NULL or Pi::LuaManager->GetLuaState();
	LuaTable(const LuaTable & ref): m_lua(ref.m_lua), m_index(ref.m_index) {} // Copy constructor.
	LuaTable(lua_State * l, int index): m_lua(l), m_index(lua_absindex(l, index)) {assert(lua_istable(m_lua, m_index));}
	explicit LuaTable(lua_State * l): m_lua(l) {
		lua_newtable(m_lua);
		m_index = lua_gettop(l);
	}

	~LuaTable() {};

	const LuaTable & operator=(const LuaTable & ref) { m_lua = ref.m_lua; m_index = ref.m_index; return *this;}
	template <class Key> void PushValueToStack(const Key & key) const;
	template <class Value, class Key> Value Get(const Key & key) const;
	template <class Key> LuaTable Sub(const Key & key) const; // Does not clean up the stack.
	template <class Value, class Key> Value Get(const Key & key, Value default_value) const;
	template <class Value, class Key> void Set(const Key & key, const Value & value) const;

	template <class Key, class Value> std::map<Key, Value> GetMap() const;
	template <class Key, class Value> void LoadMap(const std::map<Key, Value> & m) const;
	template <class Value> void LoadVector(const std::vector<Value> & m) const;

	lua_State * GetLua() const { return m_lua; }
	int GetIndex() const { return m_index; }
	int Size() const {return lua_rawlen(m_lua, m_index);}

	/* VecIter, as in VectorIterator (only shorter to type :-)
	 *
	 * Careful, its LuaTable specialization isn't stable WRT the stack, and using its value
	 * will push a table onto the stack, and will only clean it up when the iterator
	 * gets destroyed or inc/decremented.
	 *
	 * For all other values, occasional operations on the stack may occur but it should
	 * not leak anything.
	 */
	template <class Value> class VecIter : public std::iterator<std::input_iterator_tag, Value> {
		public:
			VecIter() : m_table(0), m_currentIndex(0), m_cache(), m_dirtyCache(true) {}
			~VecIter() {}
			VecIter(LuaTable * t, int currentIndex): m_table(t), m_currentIndex(currentIndex), m_cache(), m_dirtyCache(true) {}
			VecIter(const VecIter & copy): m_table(copy.m_table), m_currentIndex(copy.m_currentIndex), m_cache(), m_dirtyCache(true) {}
			void operator=(const VecIter & copy) { CleanCache(); m_table = copy.m_table; m_currentIndex = copy.m_currentIndex;}

			VecIter operator++() { if (m_table) {CleanCache(); ++m_currentIndex;} return *this; }
			VecIter operator++(int) { VecIter copy(*this); if (m_table) {CleanCache(); ++m_currentIndex;} return copy;}
			VecIter operator--() { if (m_table) --m_currentIndex; return *this; }
			VecIter operator--(int) { VecIter copy(*this); if (m_table) --m_currentIndex; return copy;}

			bool operator==(const VecIter & other) const {return (m_table == other.m_table && m_currentIndex == other.m_currentIndex);}
			bool operator!=(const VecIter & other) const {return (m_table != other.m_table || m_currentIndex != other.m_currentIndex);}
			Value operator*() { LoadCache(); return m_cache;}
			const Value * operator->() { LoadCache(); return &m_cache;}
		private:
			void CleanCache() { m_dirtyCache = true; }
			void LoadCache() {
				if (m_dirtyCache) {
					m_cache = m_table->Get<Value>(m_currentIndex);
					m_dirtyCache = false;
				}
			}
			LuaTable * m_table;
			int m_currentIndex;
			Value m_cache;
			bool m_dirtyCache;
	};
	
	template <class Value> VecIter<Value> Begin() {return VecIter<Value>(this, 1);}
	template <class Value> VecIter<Value> End() {return VecIter<Value>(this, Size()+1);}

protected:
	LuaTable(): m_lua(0), m_index(0) {} //Protected : invalid tables shouldn't be out there.
	lua_State * m_lua;
	int m_index;

};

class ScopedTable: public LuaTable {
public:
	ScopedTable(const LuaTable &t): LuaTable(t) {
		if (m_lua) {
			lua_pushvalue(m_lua, m_index);
			m_index = lua_gettop(m_lua);
		}
	}
	ScopedTable(lua_State* l): LuaTable(l) {};
	ScopedTable(const LuaRef & r): LuaTable() {
		r.PushCopyToStack();
		m_lua = r.GetLua();
		m_index = lua_gettop(m_lua);
	}
	~ScopedTable() {
		if (m_lua && !lua_isnone(m_lua, m_index) && lua_istable(m_lua, m_index))
			lua_remove(m_lua, m_index);
	}
};

#include "LuaPushPull.h"

template <class Key> void LuaTable::PushValueToStack(const Key & key) const {
	pi_lua_generic_push(m_lua, key);
	lua_gettable(m_lua, m_index);
}

template <class Key> LuaTable LuaTable::Sub(const Key & key) const {
	PushValueToStack(key);
	return (lua_istable(m_lua, -1)) ? LuaTable(m_lua, -1) : LuaTable();
}

template <class Value, class Key> Value LuaTable::Get(const Key & key) const {
	Value return_value;
	PushValueToStack(key);
	pi_lua_generic_pull(m_lua, -1, return_value);
	lua_pop(m_lua, 1);
	return return_value;
}

template <class Value, class Key> Value LuaTable::Get(const Key & key, Value default_value) const {
	PushValueToStack(key);
	if (!(lua_isnil(m_lua, -1)))
		pi_lua_generic_pull(m_lua, -1, default_value);
	lua_pop(m_lua, 1);
	return default_value;
}

template <class Value, class Key> void LuaTable::Set(const Key & key, const Value & value) const {
	pi_lua_generic_push(m_lua, key);
	pi_lua_generic_push(m_lua, value);
	lua_settable(m_lua, m_index);
}

template <class Key, class Value> std::map<Key, Value> LuaTable::GetMap() const {
	std::map<Key, Value> ret;
	lua_pushnil(m_lua);
	while(lua_next(m_lua, m_index)) {
		Key k;
		Value v;
		if (pi_lua_strict_pull(m_lua, -2, k)) {
			pi_lua_strict_pull(m_lua, -1, v);
			ret[k] = v;
		}
		lua_pop(m_lua, 1);
	}
	return ret;
}

template <class Key, class Value> void LuaTable::LoadMap(const std::map<Key, Value> & m) const {
	for (typename std::map<Key, Value>::const_iterator it = m.begin();
			it != m.end() ; ++it)
		Set(it->first, it->second);
}

template <class Value> void LuaTable::LoadVector(const std::vector<Value> & v) const {
	lua_len(m_lua, m_index);
	int current_length = lua_tointeger(m_lua, -1);
	lua_pop(m_lua, 1);
	for (unsigned int i = 0;  i < v.size() ; ++i)
		Set(current_length+i+1, v[i]);
}

template <> inline void LuaTable::VecIter<LuaTable>::LoadCache() {
	if (m_dirtyCache) {
		m_cache = m_table->Sub(m_currentIndex);
		m_dirtyCache = false;
	}
}
template <> inline void LuaTable::VecIter<LuaTable>::CleanCache() {
	if (!m_dirtyCache && m_cache.GetLua()) {
		lua_remove(m_cache.GetLua(), m_cache.GetIndex());
	}
	m_dirtyCache = true;
}
#endif
