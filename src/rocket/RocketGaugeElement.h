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

	static void Register();

	virtual Rocket::Core::String GetValue() const;
	virtual void SetValue(const Rocket::Core::String&);

	virtual float GetGaugeValue() const;
	virtual void SetGaugeValue(float val);
protected:
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnAttributeChange(const Rocket::Core::AttributeNameList& changedAttributes);
	virtual void OnPropertyChange(const Rocket::Core::PropertyNameList& changedProperties);
	virtual void ProcessEvent(Rocket::Core::Event& ev);
	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions);

	float m_value; //the actual gauge value

private:
	RocketGaugeType* type;
	Rocket::Core::String type_name;	
};