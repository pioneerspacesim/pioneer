// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include "Widget.h"

namespace UI {

class Slider: public Widget {
public:
	virtual Point PreferredSize();
	virtual void Layout();
	virtual void Draw();

	float GetValue() const { return m_value; }
	void SetValue(float v);

	void GetRange(float &out_min, float &out_max) { out_min = m_rangeMin; out_max = m_rangeMax; }
	Slider *SetRange(float min, float max);

	Slider *SetStep(float step) { m_step = step; return this; }
	void StepUp() { SetValue(m_value - m_step); }
	void StepDown() { SetValue(m_value + m_step); }

	sigc::signal<void,float> onValueChanged;

protected:
	enum SliderOrientation {
		SLIDER_HORIZONTAL,
		SLIDER_VERTICAL
	};

	Slider(Context *context, SliderOrientation orient) :
		Widget(context), m_orient(orient),
		m_rangeMin(0.0f), m_rangeMax(1.0f), m_step(0.1f), m_value(0.0f),
		m_buttonDown(false), m_mouseOverButton(false) {}

	virtual void HandleMouseDown(const MouseButtonEvent &event);
	virtual void HandleMouseUp(const MouseButtonEvent &event);
	virtual void HandleMouseMove(const MouseMotionEvent &event);
	virtual void HandleMouseOut();

private:
	void UpdateButton();
	bool PointInsideButton(const Point &p);

	SliderOrientation m_orient;
	float m_rangeMin;
	float m_rangeMax;
	float m_step;
	float m_value;
	Point m_gutterPos, m_gutterSize;
	Point m_buttonPos, m_buttonSize;
	Point m_lastMousePosition;
	bool m_buttonDown;
	bool m_mouseOverButton;
};

class HSlider: public Slider {
protected:
	friend class Context;
	HSlider(Context *context) : Slider(context, SLIDER_HORIZONTAL) {}
};

class VSlider: public Slider {
protected:
	friend class Context;
	VSlider(Context *context) : Slider(context, SLIDER_VERTICAL) {}
};

}

#endif
