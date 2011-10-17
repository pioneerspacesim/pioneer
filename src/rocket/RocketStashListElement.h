#ifndef _ROCKETSTASHLISTELEMENT_H
#define _ROCKETSTASHLISTELEMENT_H

#include "RocketManager.h"

// <list stash='...'>
//   <span stash='...'>
// </list>

class RocketStashListElement : public Rocket::Core::Element, public RocketStashConsumer< std::list<std::string> > {
public:
	RocketStashListElement(const Rocket::Core::String &_tag) : Rocket::Core::Element(_tag), m_detached(false) {}
	virtual ~RocketStashListElement();

	virtual void OnUpdate();

	static void Register();

	virtual void UpdateFromStash(const std::list<std::string> &list);

private:
	void DetachTemplate();

	bool m_detached;
	Rocket::Core::Element *m_template;
};

#endif
