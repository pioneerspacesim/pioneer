#pragma once

#include "RocketManager.h"

// <gauge type="bar" orientation="horizontal" direction="right" value="50" />
// <gauge orientation="vertical" stash="player.health" />
// <gauge type="pie" ...

class RocketGaugeType;

class RocketGaugeElement : public Rocket::Core::Element {
public:
	RocketGaugeElement(const Rocket::Core::String &tag);
	virtual ~RocketGaugeElement();

	Rocket::Core::String GetName() const;
	void SetName(const Rocket::Core::String& name);

	virtual Rocket::Core::String GetValue() const;
	virtual void SetValue(const Rocket::Core::String&);

protected:
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnAttributeChange(const Core::AttributeNameList& changed_attributes);
	virtual void OnPropertyChange(const Core::PropertyNameList& changed_properties);

	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions);

private:
	RocketGaugeType* type
	Rocket::Core::String type_name;
};