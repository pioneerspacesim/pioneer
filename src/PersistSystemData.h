#ifndef _PERSISTSYSTEMDATA_H
#define _PERSISTSYSTEMDATA_H

#include "SysLoc.h"
#include "Serializer.h"
#include <map>

template <typename T>
class PersistSystemData {
public:
	void Clear() {
		m_dict.clear();
	}
	T Get(const SysLoc &loc, T defaultValIfNotExists) const {
		typename std::map<SysLoc, T>::const_iterator i = m_dict.find(loc);
		if (i == m_dict.end()) return defaultValIfNotExists;
		else return (*i).second;
	}
	void Set(const SysLoc &loc, T val) {
		m_dict[loc] = val;
	}
	void Serialize(Serializer::Writer &wr) const {
		wr.Int32(m_dict.size());
		for (typename std::map<SysLoc, T>::const_iterator i = m_dict.begin(); i != m_dict.end(); ++i) {
			(*i).first.Serialize(wr);
			wr.Auto((*i).second);
		}
	}
	static void Unserialize(Serializer::Reader &rd, PersistSystemData<T> *pd) {
		int num = rd.Int32();
		while (num-- > 0) {
			SysLoc loc;
			SysLoc::Unserialize(rd, &loc);
			T val;
			rd.Auto(&val);
			pd->m_dict[loc] = val;
		}
	}
private:
	std::map<SysLoc, T> m_dict;
};

#endif /* _PERSISTSYSTEMDATA_H */
