#ifndef _UI_BACKGROUND_H
#define _UI_BACKGROUND_H

#include "Single.h"

namespace UI {

class Background : public Single {
public:
	virtual vector2f PreferredSize();
	virtual void Layout();
	virtual void Draw();

protected:
	friend class Context;
	Background(Context *context) : Single(context) {}
};

}

#endif
