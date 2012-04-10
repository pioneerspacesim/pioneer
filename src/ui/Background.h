#ifndef _UI_BACKGROUND_H
#define _UI_BACKGROUND_H

#include "Single.h"
#include "Color.h"

namespace UI {

class Background : public Single {
public:
	virtual void Draw();

protected:
	friend class Context;
	Background(Context *context, const Color &color) : Single(context), m_color(color) {}

private:
	Color m_color;
};

}

#endif
