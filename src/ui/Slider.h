#ifndef _UI_SLIDER_H
#define _UI_SLIDER_H

#include "Widget.h"

namespace UI {

class Slider: public Widget {
public:
	virtual vector2f PreferredSize();
	virtual void Layout();
	virtual void Draw();

	float GetValue() const { return m_value; }
	void SetValue(float v);

protected:
	enum SliderOrientation {
		SLIDER_HORIZONTAL,
		SLIDER_VERTICAL
	};

	Slider(Context *context, SliderOrientation orient) : Widget(context), m_orient(orient), m_value(0.0f), m_buttonDown(false) {}

	virtual bool HandleMouseDown(const MouseButtonEvent &event, bool emit);
	virtual bool HandleMouseUp(const MouseButtonEvent &event, bool emit);
	virtual bool HandleMouseMove(const MouseMotionEvent &event, bool emit);

private:
	void UpdateButton();

	SliderOrientation m_orient;
	float m_value;
	vector2f m_buttonPos, m_buttonSize;
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
