#pragma once

#include "RocketManager.h"

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
	virtual bool OnAttributeChange(const Rocket::Core::AttributeNameList& changedAttributes);
	virtual void OnPropertyChange(const Rocket::Core::PropertyNameList& changedProperties);
	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions) = 0;
protected:
	RocketGaugeElement* parent;
};