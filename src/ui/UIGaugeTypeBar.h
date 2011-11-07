#ifndef _UIGAUGETYPEBAR_H
#define _UIGAUGETYPEBAR_H

#include "UIGaugeType.h"

namespace UI {

/*
 * Horizontal or vertical "progress" bar
 */
class GaugeTypeBar : public GaugeType
{
public:
	enum Direction /* Fill direction */
	{
		RIGHT,
		LEFT,
		UP,
		DOWN
	};
	GaugeTypeBar(GaugeElement *element);
	virtual ~GaugeTypeBar();

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

}

#endif
