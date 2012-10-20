// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
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

	sigc::signal<void,float> onValueChanged;

protected:
	enum SliderOrientation {
		SLIDER_HORIZONTAL,
		SLIDER_VERTICAL
	};

	Slider(Context *context, SliderOrientation orient) : Widget(context), m_orient(orient), m_value(0.0f), m_buttonDown(false) {}

	virtual void HandleMouseDown(const MouseButtonEvent &event);
	virtual void HandleMouseUp(const MouseButtonEvent &event);
	virtual void HandleMouseMove(const MouseMotionEvent &event);

private:
	void UpdateButton();

	SliderOrientation m_orient;
	float m_value;
	Point m_buttonPos, m_buttonSize;
	bool m_buttonDown;
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
