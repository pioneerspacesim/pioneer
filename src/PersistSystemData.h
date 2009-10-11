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
	void Serialize() const {
		using namespace Serializer::Write;
		wr_int(m_dict.size());
		for (typename std::map<SysLoc, T>::const_iterator i = m_dict.begin(); i != m_dict.end(); ++i) {
			(*i).first.Serialize();
			wr_auto((*i).second);
		}
	}
	static void Unserialize(PersistSystemData<T> *pd) {
		using namespace Serializer::Read;
		int num = rd_int();
		while (num-- > 0) {
			SysLoc loc;
			SysLoc::Unserialize(&loc);
			T val;
			rd_auto(&val);
			pd->m_dict[loc] = val;
		}
	}
private:
	std::map<SysLoc, T> m_dict;
};

#endif /* _PERSISTSYSTEMDATA_H */
