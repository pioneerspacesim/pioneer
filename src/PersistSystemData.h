#ifndef _PERSISTSYSTEMDATA_H
#define _PERSISTSYSTEMDATA_H

#include "Serializer.h"
#include "galaxy/SystemPath.h"
#include <map>

template <typename T>
class PersistSystemData {
public:
	void Clear() {
		m_dict.clear();
	}
	T Get(const SystemPath &path, T defaultValIfNotExists) const {
		typename std::map<SystemPath, T>::const_iterator i = m_dict.find(path.SystemOnly());
		if (i == m_dict.end()) return defaultValIfNotExists;
		else return (*i).second;
	}
	void Set(const SystemPath &path, T val) {
		m_dict[path.SystemOnly()] = val;
	}
	void Serialize(Serializer::Writer &wr) const {
		wr.Int32(m_dict.size());
		for (typename std::map<SystemPath, T>::const_iterator i = m_dict.begin(); i != m_dict.end(); ++i) {
			(*i).first.Serialize(wr);
			wr.Auto((*i).second);
		}
	}
	static void Unserialize(Serializer::Reader &rd, PersistSystemData<T> *pd) {
		int num = rd.Int32();
		while (num-- > 0) {
			SystemPath path = SystemPath::Unserialize(rd);
			T val;
			rd.Auto(&val);
			pd->m_dict[path] = val;
		}
	}
private:
	std::map<SystemPath, T> m_dict;
};

#endif /* _PERSISTSYSTEMDATA_H */
