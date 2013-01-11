// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PROPERTYHOLDER_H
#define PROPERTYHOLDER_H

#include <string>
#include <map>
#include <cassert>

struct lua_State;

template <typename T>
class TypedPropertyHolder {
protected:
	TypedPropertyHolder() {}

	typedef std::map<std::string,T> PropertyMap;
	typedef typename PropertyMap::const_iterator PropertyMapIterator;

	void DeclareProperty(const std::string &k) {
		typename PropertyMap::iterator i;
		assert(!GetIterator(k, i));
		m_props.insert(i, typename PropertyMap::value_type(k, T()));
	}

	void SetProperty(const std::string &k, const T &v) {
		typename PropertyMap::iterator i;
		assert(GetIterator(k, i));
		i->second = v;
	}

	const T &GetProperty(const std::string &k) {
		typename PropertyMap::iterator i;
		assert(GetIterator(k, i));
		return i->second;
	}

	bool GetIterator(const std::string &k, typename PropertyMap::iterator &i)
	{
		i = m_props.lower_bound(k);
		return (i != m_props.end() && !(m_props.key_comp()(k, i->first))); // key must exist already
	}

	bool PushPropertyToLua(lua_State *l, const std::string &k);
	void AddPropertiesToLuaTable(lua_State *l, int tableIdx);

	PropertyMapIterator PropertiesBegin() const { return m_props.begin(); }
	PropertyMapIterator PropertiesEnd() const { return m_props.end(); }

private:
	PropertyMap m_props;
};

class PropertyHolder :
	public TypedPropertyHolder<bool>,
	public TypedPropertyHolder<int>,
	public TypedPropertyHolder<double>,
	public TypedPropertyHolder<std::string>
{
public:
	void SetProperty(const std::string &k, bool v)               { TypedPropertyHolder<bool>::SetProperty(k, v); }
	void SetProperty(const std::string &k, int v)                { TypedPropertyHolder<int>::SetProperty(k, v); }
	void SetProperty(const std::string &k, double v)             { TypedPropertyHolder<double>::SetProperty(k, v); }
	void SetProperty(const std::string &k, const std::string &v) { TypedPropertyHolder<std::string>::SetProperty(k, v); }

	void GetProperty(const std::string &k, bool &v)        { v = TypedPropertyHolder<bool>::GetProperty(k); }
	void GetProperty(const std::string &k, int &v)         { v = TypedPropertyHolder<int>::GetProperty(k); }
	void GetProperty(const std::string &k, double &v)      { v = TypedPropertyHolder<double>::GetProperty(k); }
	void GetProperty(const std::string &k, std::string &v) { v = TypedPropertyHolder<std::string>::GetProperty(k); }

	bool PushPropertyToLua(lua_State *l, const std::string &k);
	void AddPropertiesToLuaTable(lua_State *l, int tableIdx);

protected:
	PropertyHolder() {}

	void DeclareBooleanProperty(const std::string &k) { TypedPropertyHolder<bool>::DeclareProperty(k); }
	void DeclareIntegerProperty(const std::string &k) { TypedPropertyHolder<int>::DeclareProperty(k); }
	void DeclareFloatProperty(const std::string &k)   { TypedPropertyHolder<double>::DeclareProperty(k); }
	void DeclareStringProperty(const std::string &k)  { TypedPropertyHolder<std::string>::DeclareProperty(k); }
};

#endif
