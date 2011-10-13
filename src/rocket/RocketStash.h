#ifndef _ROCKETSTASH_H
#define _ROCKETSTASH_H

#include <map>
#include <tr1/memory>

#include "Body.h"
#include "ShipFlavour.h"

#include "Rocket/Core/Element.h"


class RocketBodyConsumer {
public:
	virtual void UpdateBody(const Body *body) = 0;
};

class RocketShipFlavourConsumer {
public:
	virtual void UpdateShipFlavour(const ShipFlavour &flavour) = 0;
};


class RocketStash {
public:
	RocketStash() : m_needsStashUpdate(false) {}

	void SetStashItem(const std::string &id, const std::string &value);
	//void SetStashItem(const std::string &id, const ShipFlavour &value);
	//void SetStashItem(const std::string &id, const Body *value);

	void ClearStashItem(const std::string &id);
	void ClearStash();

	void Update(Rocket::Core::Element *e);

private:
	class StashItem {
	public:
		virtual void Update(Rocket::Core::Element *e) = 0;
	};

	class StashStringItem : public StashItem {
	public:
		StashStringItem(const std::string &value) : m_value(value) {}
		virtual void Update(Rocket::Core::Element *e);
	private:
		std::string m_value;
	};

	class StashBodyItem : public StashItem {
	public:
		StashBodyItem(const Body *value) : m_value(value) {}
		virtual void Update(Rocket::Core::Element *e);
	private:
		const Body *m_value;
	};

	class StashShipFlavourItem : public StashItem {
	public:
		StashShipFlavourItem(ShipFlavour value) : m_value(value) {}
		virtual void Update(Rocket::Core::Element *e);
	private:
		ShipFlavour m_value;
	};

	typedef std::tr1::shared_ptr<StashItem> StashItemPtr;
	
	std::map<std::string,StashItemPtr> m_stash;
	bool m_needsStashUpdate;
};

#endif
