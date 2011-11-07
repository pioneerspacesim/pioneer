#ifndef _UISTASH_H
#define _UISTASH_H

#include <map>
#ifdef _MSC_VER
#include <memory>
#else
#include <tr1/memory>
#endif

#include "Rocket/Core/Element.h"

namespace UI {

template <typename T>
class StashConsumer {
public:
	virtual void UpdateFromStash(const T &value) = 0;
};


class Stash {
public:
	Stash() : m_needsStashUpdate(false) {}

	template <typename T>
	void SetStashItem(const std::string &id, const T &value) {
		StashItemPtr item(new StashItem<T>(value));
		m_stash[id] = item;
		m_needsStashUpdate = true;
	}

	void SetStashItem(const std::string &id, char value[]) {
		SetStashItem(id, std::string(value));
	}

	void ClearStashItem(const std::string &id);
	void ClearStash();

	void Update(Rocket::Core::Element *e, bool force = false);

private:
	class StashItemBase {
	public:
		virtual void UpdateElement(Rocket::Core::Element *e) = 0;
	};

	template <typename T>
	class StashItem : public StashItemBase {
	public:
		StashItem(const T &value) : m_value(value) {}
		virtual void UpdateElement(Rocket::Core::Element *e) {
			StashConsumer<T> *c = dynamic_cast<StashConsumer<T>*>(e);
			if (!c) return;
			c->UpdateFromStash(m_value);
		}
	
	private:
		T m_value;
	};

	typedef std::tr1::shared_ptr<StashItemBase> StashItemPtr;
	
	std::map<std::string,StashItemPtr> m_stash;
	bool m_needsStashUpdate;
};

template <>
class Stash::StashItem<std::string> : public StashItemBase {
public:
	StashItem(const std::string &value) : m_value(value) {}
	virtual void UpdateElement(Rocket::Core::Element *e) {
		e->SetInnerRML(m_value.c_str());
	}

private:
	std::string m_value;
};

}

#endif
