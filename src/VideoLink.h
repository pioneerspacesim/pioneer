#ifndef _VIDEOLINK_H
#define _VIDEOLINK_H

#include "gui/Gui.h"

class VideoLink : public Gui::Widget {
public:
	VideoLink(float w, float h) : m_width(w), m_height(h) {}

	virtual void GetSizeRequested(float size[2]) {
		size[0] = m_width;
		size[1] = m_height;
	}

private:
	float m_width, m_height;
};

#endif
