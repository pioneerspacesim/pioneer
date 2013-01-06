// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _REFLIST_H
#define _REFLIST_H

#include <map>
#include <list>

template <typename T>
struct RefItem {
	int ref;
};

template <typename T>
class RefList {
public:
	RefList() : m_next_ref(0), m_dirty(true) {}

	inline int Add(RefItem<T> &ro) {
		T *o = static_cast<T*>(&ro);
		o->ref = ++m_next_ref;
		m_objects.insert( std::make_pair(o->ref, *o) );
		m_dirty = true;
		return o->ref;
	}
	inline void Update(int ref, RefItem<T> &ro) {
		assert(ref == ro.ref);
		T *o = static_cast<T*>(&ro);
		m_objects.erase(ref);
		m_objects.insert( std::make_pair(ref, *o) );
		m_dirty = true;
	}
	inline void Remove(int ref) {
		m_objects.erase(ref);
		m_dirty = true;
	}
	inline const T *Get(int ref) {
		typename std::map<int,T>::iterator i = m_objects.find(ref);
		if (i == m_objects.end())
			return NULL;
		return const_cast<const T*>(&((*i).second));
	}
	inline const std::list<const T*> &GetAll() {
		if (!m_dirty) return m_object_list;
		m_object_list.clear();
		for (typename std::map<int,T>::iterator i = m_objects.begin(); i != m_objects.end(); ++i)
			m_object_list.push_back(const_cast<const T*>(&((*i).second)));
		m_dirty = false;
		return m_object_list;
	}

private:
	RefList(const RefList &) {}

	std::map<int,T> m_objects;
	int m_next_ref;

	std::list<const T*> m_object_list;
	bool m_dirty;
};

#endif
