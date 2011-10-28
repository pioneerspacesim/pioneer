#ifndef _ROCKETGAUGETYPEBAR_H
#define _ROCKETGAUGETYPEBAR_H

#include "RocketGaugeType.h"

/*
 * Horizontal or vertical "progress" bar
 */
class RocketGaugeTypeBar : public RocketGaugeType
{
public:
	enum Direction /* Fill direction */
	{
		RIGHT,
		LEFT,
		UP,
		DOWN
	};
	RocketGaugeTypeBar(RocketGaugeElement *element);
	virtual ~RocketGaugeTypeBar();

	bool Initialize();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual bool OnAttributeChange(const Rocket::Core::AttributeNameList &changedAttributes);
	virtual void ProcessEvent(Rocket::Core::Event &event);
	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f &dimensions);
	virtual void OnValueChanged();

protected:
	void FormatElements();

private:
	Rocket::Core::Element *m_bar;
	Direction m_direction;
};

#endif