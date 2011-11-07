#ifndef _UIGAUGETYPE_H
#define _UIGAUGETYPE_H

#include "UIManager.h"

namespace UI {

class GaugeElement;

/*
 * GaugeType base class
 * Gauge types do the actual drawing & important stuff
 */
class GaugeType
{
public:
	GaugeType(GaugeElement *element);
	virtual ~GaugeType();

	virtual Rocket::Core::String GetValue() const;
	virtual void OnUpdate();
	virtual void OnRender();
	virtual bool OnAttributeChange(const Rocket::Core::AttributeNameList &changedAttributes);
	virtual void OnPropertyChange(const Rocket::Core::PropertyNameList &changedProperties);
	virtual void ProcessEvent(Rocket::Core::Event &event) = 0;
	virtual bool GetIntrinsicDimensions(Rocket::Core::Vector2f &dimensions) = 0;
	virtual void OnValueChanged();

protected:
	GaugeElement* m_parent;
};

}

#endif
