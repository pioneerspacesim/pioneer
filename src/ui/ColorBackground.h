#ifndef _UI_COLORBACKGROUND_H
#define _UI_COLORBACKGROUND_H

#include "Single.h"
#include "Color.h"

namespace UI {

class ColorBackground : public Single {
public:
	virtual void Draw();

	void SetColor(const Color &color) { m_color = color; }

protected:
	friend class Context;
	ColorBackground(Context *context, const Color &color) : Single(context), m_color(color) {}

private:
	Color m_color;
};

}

#endif
