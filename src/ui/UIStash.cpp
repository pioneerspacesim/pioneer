#include "UIStash.h"

#include <queue>

namespace UI {

void Stash::ClearStashItem(const std::string &id)
{
	m_stash.erase(id);
}

void Stash::ClearStash()
{
	m_stash.clear();
}

void Stash::Update(Rocket::Core::Element *rootElement, bool force)
{
	if (!force && !m_needsStashUpdate) return;

	std::queue<Rocket::Core::Element*> searchQueue;
	searchQueue.push(rootElement);

	while (!searchQueue.empty()) {
		Rocket::Core::Element *e = searchQueue.front();
		searchQueue.pop();

		Rocket::Core::String stash = e->GetAttribute<Rocket::Core::String>("stash", "");
		if (stash.Length() > 0) {

			std::map<std::string,StashItemPtr>::iterator i = m_stash.find(stash.CString());
			if (i != m_stash.end())
				(*i).second->UpdateElement(e);
		}

		for (int i=0; i < e->GetNumChildren(); i++)
			searchQueue.push(e->GetChild(i));
	}

	m_needsStashUpdate = false;
}

}
