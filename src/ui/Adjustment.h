// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_ADJUSTMENT_H
#define UI_ADJUSTMENT_H

#include "Container.h"



namespace UI {

class HBox;
class HSlider;

class Adjustment : public Container {
public:
	virtual Point PreferredSize();
	virtual void Layout();

	Adjustment *SetInnerWidget(Widget *widget);
	void RemoveInnerWidget();
	Widget *GetInnerWidget() const { return m_innerWidget; }

	float GetScrollPosition() const;
	void SetScrollPosition(float v);
	UI::HSlider *GetSlider() const { return m_slider; }
	void SetRange(std::pair<float,float> a) { m_range = a; }
	
	sigc::signal<void> onSliderChanged;

protected:
	friend class Context;
	Adjustment(Context *context) : Container(context), m_innerWidget(0), m_slider(0), m_pos(0.0f), m_range(0.0f,1.0f) {}

	virtual void RemoveWidget(Widget *widget);

private:
	Widget *m_innerWidget;
	HSlider *m_slider;
	
	float m_pos;
	std::pair<float,float> m_range;

	sigc::connection m_onMouseWheelConn;

	void OnScroll(float value);
// 	bool OnMouseUp(const MouseButtonEvent);
	bool OnMouseWheel(const MouseWheelEvent &event);
	template<typename Tin, typename Tout>
	Tout ConvertToRange(const std::pair<Tin,Tin> sourcePair, const std::pair<Tout, Tout> destPair, const Tin input) const
	{
		//linear interpolation based on formula from http://mathforum.org/library/drmath/view/60433.html
		Tin normPosition = ((input - sourcePair.first) / (sourcePair.second - sourcePair.first));
		Tout destPairNorm = normPosition * (destPair.second - destPair.first);
		
		return (destPair.first + destPairNorm);
	}
};

}

#endif
