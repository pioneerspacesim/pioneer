#pragma once

#include "RocketGaugeType.h"

/*
 * Horizontal or vertical "progress" bar
 */
class RocketGaugeTypeBar : public RocketGaugeType
{
public:
	enum Orientation
	{
		VERTICAL,
		HORIZONTAL
	};
	RocketGaugeTypeBar(RocketGaugeElement* element);
	virtual ~RocketGaugeTypeBar();

	bool Initialize();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual bool OnAttributeChange(const Rocket::Core::AttributeNameList& changedAttributes);
	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f& dimensions);
protected:
	void FormatElements(const Rocket::Core::Vector2f& containingBlock, float length);

private:
	Rocket::Core::Element* background;
	Rocket::Core::Element* bar;
	Orientation orientation;
};