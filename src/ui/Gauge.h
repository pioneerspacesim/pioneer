// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_GAUGE_H
#define UI_GAUGE_H

#include "Widget.h"

namespace UI {

class Gauge: public Widget {
public:
	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

	float GetValue() const { return m_value; }
	void SetValue(float v) { m_value = Clamp(v, 0.0f, 1.0f); }

protected:
	friend class Context;
	Gauge(Context *context): Widget(context), m_value(0.0) {}

private:
	float m_value;
};

}

#endif
