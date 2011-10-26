#pragma once

#include "RocketGaugeType.h"

/*
 * Horizontal or vertical "progress" bar
 */
class RocketGaugeTypeBar : public RocketGaugeType
{
public:
	RocketGaugeTypeBar(RocketGaugeElement* element);
	virtual ~RocketGaugeTypeBar();

	bool Initialize();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual bool OnAttributeChange(const Core::AttributeNameList& changed_attributes);
	virtual void OnPropertyChange(const Core::PropertyNameList& changed_properties);
	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions) = 0;
protected:
	void FormatElements(const Rocket::Core::Vector2f& containing_block, float bar_length);
	Rocket::Core::Element* background;
	Rocket::Core::Element* bar;
};