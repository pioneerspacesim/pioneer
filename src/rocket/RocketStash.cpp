#include "RocketStash.h"

#include <queue>

void RocketStash::StashStringItem::Update(Rocket::Core::Element *e)
{
	e->SetInnerRML(m_value.c_str());
}

void RocketStash::StashBodyItem::Update(Rocket::Core::Element *e)
{
	RocketBodyConsumer *c = dynamic_cast<RocketBodyConsumer*>(e);
	if (!c) return;
	c->UpdateBody(m_value);
}

void RocketStash::StashShipFlavourItem::Update(Rocket::Core::Element *e)
{
	RocketShipFlavourConsumer *c = dynamic_cast<RocketShipFlavourConsumer*>(e);
	if (!c) return;
	c->UpdateShipFlavour(m_value);
}


void RocketStash::SetStashItem(const std::string &id, const std::string &value)
{
	StashItemPtr item(new StashStringItem(value));
	m_stash[id] = item;
	m_needsStashUpdate = true;
}

void RocketStash::SetStashItem(const std::string &id, Body *value)
{
	StashItemPtr item(new StashBodyItem(value));
	m_stash[id] = item;
	m_needsStashUpdate = true;
}

void RocketStash::SetStashItem(const std::string &id, const ShipFlavour &value)
{
	StashItemPtr item(new StashShipFlavourItem(value));
	m_stash[id] = item;
	m_needsStashUpdate = true;
}

void RocketStash::ClearStashItem(const std::string &id)
{
	m_stash.erase(id);
}

void RocketStash::ClearStash()
{
	m_stash.clear();
}

void RocketStash::Update(Rocket::Core::Element *rootElement)
{
	if (!m_needsStashUpdate) return;

	std::queue<Rocket::Core::Element*> searchQueue;
	searchQueue.push(rootElement);

	while (!searchQueue.empty()) {
		Rocket::Core::Element *e = searchQueue.front();
		searchQueue.pop();

		Rocket::Core::String stash = e->GetAttribute<Rocket::Core::String>("stash", "");
		if (stash.Length() > 0) {

			std::map<std::string,StashItemPtr>::iterator i = m_stash.find(stash.CString());
			if (i != m_stash.end())
				(*i).second->Update(e);
		}

		for (int i=0; i < e->GetNumChildren(); i++)
			searchQueue.push(e->GetChild(i));
	}

	m_needsStashUpdate = false;
}
