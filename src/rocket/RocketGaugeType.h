#pragma once

class RocketGaugeElement;

/*
 * GaugeType base class
 * Gauge types do the actual drawing & important stuff
 */
class RocketGaugeType
{
public:
	RocketGaugeType(RocketGaugeElement* element);
	virtual ~RocketGaugeType();

	virtual Rocket::Core::String GetValue() const;
	virtual void OnUpdate();
	virtual void OnRender();
	virtual bool OnAttributeChange(const Core::AttributeNameList& changed_attributes);
	virtual void OnPropertyChange(const Core::PropertyNameList& changed_properties);
	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions) = 0;
protected:
	RocketGaugeElement* parent;
};