#include "RocketStashListElement.h"

RocketStashListElement::~RocketStashListElement()
{
}

void RocketStashListElement::DetachTemplate()
{
	assert(!m_detached);

	m_template = GetOwnerDocument()->CreateElement("template");

	while (HasChildNodes())
		m_template->AppendChild(GetFirstChild());

	m_detached = true;
}

void RocketStashListElement::OnUpdate()
{
	if (!m_detached)
		DetachTemplate();
}

void RocketStashListElement::UpdateFromStash(const std::list<std::string> &list)
{
	if (!m_detached)
		DetachTemplate();
	
	const Rocket::Core::String stashName(GetAttribute<Rocket::Core::String>("stash", ""));
	assert(stashName.Length() > 0);
	
	while (HasChildNodes())
		RemoveChild(GetFirstChild());

	for (std::list<std::string>::const_iterator i = list.begin(); i != list.end(); i++) {

		std::queue<Rocket::Core::Element*> searchQueue;
	
		for (int j=0; j < m_template->GetNumChildren(); j++) {
			Rocket::Core::Element *orig = m_template->GetChild(j);

			// XXX clone is slow and doesn't the toplevel attributes so we
			//     need to do those ourselves. method really needs a rewrite
			//     in rocket proper
			Rocket::Core::Element *e = orig->Clone();

			int index = 0;
			Rocket::Core::String key, value;

			while (orig->IterateAttributes(index, key, value))
				e->SetAttribute(key, value);

			AppendChild(e);
			e->RemoveReference();

			searchQueue.push(e);
		}

		while (!searchQueue.empty()) {
			Rocket::Core::Element *e = searchQueue.front();
			searchQueue.pop();

			if (e->GetAttribute<Rocket::Core::String>("stash", "") == stashName) {
				e->SetInnerRML((*i).c_str());
				e->RemoveAttribute("stash");
			}

			for (int j=0; j < e->GetNumChildren(); j++)
				searchQueue.push(e->GetChild(j));
		}
	}
}

class RocketStashListElementInstancer : public Rocket::Core::ElementInstancer {
	virtual Rocket::Core::Element *InstanceElement(Rocket::Core::Element *parent, const Rocket::Core::String &tag, const Rocket::Core::XMLAttributes &attributes) {
		return new RocketStashListElement(tag);
	}

	virtual void ReleaseElement(Rocket::Core::Element *element) {
		delete element;
	}

	virtual void Release() {
		delete this;
	}
};

void RocketStashListElement::Register() {
	Rocket::Core::ElementInstancer *instancer = new RocketStashListElementInstancer();
	Rocket::Core::Factory::RegisterElementInstancer("list", instancer);
	instancer->RemoveReference();
}
