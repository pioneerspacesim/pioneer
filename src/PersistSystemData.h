// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
	void ToJson(Json::Value &jsonObj) const {
		Json::Value dictArray(Json::arrayValue); // Create JSON array to contain dict data.
		for (typename std::map<SystemPath, T>::const_iterator i = m_dict.begin(); i != m_dict.end(); ++i)
		{
			Json::Value dictArrayEl(Json::objectValue); // Create JSON object to contain dict element.
			(*i).first.ToJson(dictArrayEl);
			dictArrayEl["value"] = AutoToStr((*i).second);
			dictArray.append(dictArrayEl); // Append dict object to array.
		}
		jsonObj["dict"] = dictArray; // Add dict array to supplied object.
	}
	static void FromJson(const Json::Value &jsonObj, PersistSystemData<T> *pd) {
		if (!jsonObj.isMember("dict")) throw SavedGameCorruptException();
		Json::Value dictArray = jsonObj["dict"];
		if (!dictArray.isArray()) throw SavedGameCorruptException();
		for (unsigned int arrayIndex = 0; arrayIndex < dictArray.size(); ++arrayIndex)
		{
			Json::Value dictArrayEl = dictArray[arrayIndex];
			if (!dictArrayEl.isMember("value")) throw SavedGameCorruptException();
			SystemPath path = SystemPath::FromJson(dictArrayEl);
			T val;
			StrToAuto(&val, dictArrayEl["value"].asString());
			pd->m_dict[path] = val;
		}
	}
private:
	std::map<SystemPath, T> m_dict;
};

#endif /* _PERSISTSYSTEMDATA_H */
