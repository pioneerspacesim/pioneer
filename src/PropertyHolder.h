// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef PROPERTYHOLDER_H
#define PROPERTYHOLDER_H

#include <string>
#include <map>
#include <cassert>

template <typename T>
class TypedPropertyHolder {
protected:
	TypedPropertyHolder() {}

	void DeclareProperty(const std::string &k) {
		typename PropertyMap::iterator i = m_props.lower_bound(k);
		assert(i == m_props.end() || m_props.key_comp()(k, i->first)); // key must not exist
		m_props.insert(i, typename PropertyMap::value_type(k, T()));
	}

	void SetProperty(const std::string &k, const T &v) {
		typename PropertyMap::iterator i = m_props.lower_bound(k);
		assert(i != m_props.end() && !(m_props.key_comp()(k, i->first))); // key must exist already
		i->second = v;
	}

	const T &GetProperty(const std::string &k) {
		typename PropertyMap::iterator i = m_props.lower_bound(k);
		assert(i != m_props.end() && !(m_props.key_comp()(k, i->first))); // key must exist already
		return i->second;
	}

private:
	typedef std::map<std::string,T> PropertyMap ;
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

protected:
	PropertyHolder() {}

	void DeclareBooleanProperty(const std::string &k) { TypedPropertyHolder<bool>::DeclareProperty(k); }
	void DeclareIntegerProperty(const std::string &k) { TypedPropertyHolder<int>::DeclareProperty(k); }
	void DeclareFloatProperty(const std::string &k)   { TypedPropertyHolder<double>::DeclareProperty(k); }
	void DeclareStringProperty(const std::string &k)  { TypedPropertyHolder<std::string>::DeclareProperty(k); }
};

#endif
