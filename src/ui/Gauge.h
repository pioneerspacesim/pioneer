// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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

	float GetValue() const { return m_value/m_mult; }
	void SetValue(float v);

	Gauge *SetUpperValue(float v);

	Gauge *SetWarningLevel(float v);
	Gauge *SetCriticalLevel(float v);
	Gauge *SetLevelAscending(bool ascending);

protected:
	friend class Context;
	Gauge(Context *context);

private:
	void UpdateStyle();

	void BindValue(PropertyMap &p, const std::string &k);
	void BindValuePercent(PropertyMap &p, const std::string &k);

	float m_value;

	float m_warningLevel;
	float m_criticalLevel;
	bool m_levelAscending;

	float m_mult;

	enum {
		NORMAL,
		WARNING,
		CRITICAL
	} m_style;
};

}

#endif
