// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _LUATABLE_H
#define _LUATABLE_H

#include <cassert>
#include <iterator>

#include "lua/lua.hpp"
#include "LuaRef.h"
#include "LuaPushPull.h"

/*
 * The LuaTable class is a wrapper around a table present on the stack. There
 * are two ways to instantiate a LuaTable object:
 *
 * > lua_State *l;
 * > int i; // the stack index of a table
 * > LuaTable(l); // This will allocate a new table on top of the stack
 * > LuaTable(l, i); // This will wrap the object around an existing table
 *
 * Note that the LuaTable object never removes the wrapped table from the stack.
 * Also, there is no integrity check except at the object creation, which means
 * that if you fiddle with the stack below the wrapped table you will get
 * unexpected results (most likely a crash).
 *
 * Get/Set:
 *
 * The Get and Set operators use the pi_lua_generic_{push pull} functions
 * to fetch a value of any type from the table. It is possible to add support
 * for extra types by overloading the aforementioned functions for the new
 * type. The Get function takes an optional default value, which will be returned
 * if the table does not contain the requested key. If no default is given, and
 * the key is not in the table then a Lua error is generated. If the key is present
 * but the value has an incompatible type, then a Lua error is generated (even if
 * a default value is passed to the Get method).
 *
 * These operations are designed to restore the state of the stack, thus making
 * it impossible to Get a LuaTable, as the latter would need a place on the
 * stack. For this reason, the Sub() method is used to get a subtable,
 * placing the "returned" table on the top of the stack.
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
 *
 * STL loaders:
 *
 * If you want to load a whole vector or map into a LuaTable, just do
 *
 * > std::vector v; // Or std::list, std::set, whatever as long as it has iterators
 * > std::map m;
 * > LuaTable t;
 * > T.LoadMap(m.begin(), m.end());
 * > T.LoadVector(v.begin(), v.end());
 *
 * Note that LoadVector doesn't overwrite the content of the table, it appends
 * to its array-like part. Unless you have numerical index beyond its length,
 * which you shouldn't do anyway.
 *
 * LoadMap happily overwrites any data if necessary.
 *
 * VecIter:
 *
 * It is possible to get STL-like iterators on the array part of the table.
 * The use cases are typically in loops or to use in the STL algorithms or as
 * inputs for containers.
 *
 * The two methods are LuaTable::Begin<Value>() and LuaTable::End<Value>()
 *
 * As usual, since C++ is static typed, the iterators will fail in a mixed-typed table
 * (generating a Lua error when you attempt to access an element with the wrong type).
 *
 * ScopedTable:
 *
 * The ScopedTable class is a LuaTable derivative that comes with two constructors:
 *   * New table constructor: ScopedTable(l);
 *   * LuaRef contructor: ScopedTable(my_lua_ref_object);
 * Both constructors will push a new table onto the stack, and when the C++
 * ScopedTable objects are destroyed, this new table is removed and everything
 * above it on the stack gets shifted down.
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

	~LuaTable() {}

	const LuaTable & operator=(const LuaTable & ref) { m_lua = ref.m_lua; m_index = ref.m_index; return *this;}
	template <class Key> void PushValueToStack(const Key & key) const;
	template <class Value, class Key> Value Get(const Key & key) const;
	template <class Key> LuaTable Sub(const Key & key) const; // Does not clean up the stack.
	template <class Value, class Key> Value Get(const Key & key, Value default_value) const;
	template <class Value, class Key> void Set(const Key & key, const Value & value) const;

	template <class PairIterator> void LoadMap(PairIterator beg, PairIterator end) const;
	template <class ValueIterator> void LoadVector(ValueIterator beg, ValueIterator end) const;

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
	ScopedTable(lua_State* l): LuaTable(l) {}
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

template <class PairIterator> void LuaTable::LoadMap(PairIterator beg, PairIterator end) const {
	for (PairIterator it = beg; it != end ; ++it)
		Set(it->first, it->second);
}

template <class ValueIterator> void LuaTable::LoadVector(ValueIterator beg, ValueIterator end) const {
	lua_len(m_lua, m_index);
	int i = lua_tointeger(m_lua, -1) + 1;
	lua_pop(m_lua, 1);
	for (ValueIterator it = beg;  it != end ; ++it, ++i)
		Set(i, *it);
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
