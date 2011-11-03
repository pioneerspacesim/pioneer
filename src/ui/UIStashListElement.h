#ifndef _UISTASHLISTELEMENT_H
#define _UISTASHLISTELEMENT_H

#include "UIManager.h"

// <list stash='...'>
//   <span stash='...'>
// </list>

namespace UI {

class StashListElement : public Rocket::Core::Element, public StashConsumer< std::list<std::string> > {
public:
	StashListElement(const Rocket::Core::String &_tag) : Rocket::Core::Element(_tag), m_detached(false) {}
	virtual ~StashListElement();

	virtual void OnUpdate();

	static void Register();

	virtual void UpdateFromStash(const std::list<std::string> &list);

private:
	void DetachTemplate();

	bool m_detached;
	Rocket::Core::Element *m_template;
};

}

#endif
