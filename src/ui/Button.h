#ifndef _UI_BUTTON_H
#define _UI_BUTTON_H

#include "Single.h"

namespace UI {

class Button: public Single {
public:
	virtual Metrics GetMetrics(const vector2f &hint);
	virtual void Layout();
	virtual void Draw();

protected:
	friend class Context;
	Button(Context *context): Single(context), m_active(false) {}

	friend class EventDispatcher;

	virtual void Activate();
	virtual void Deactivate();

private:
	bool m_active;
};

}

#endif
