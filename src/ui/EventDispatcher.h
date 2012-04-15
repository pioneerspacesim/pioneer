#ifndef _UI_EVENTDISPATCHER_H
#define _UI_EVENTDISPATCHER_H

#include "Event.h"

namespace UI {

class Widget;

class EventDispatcher {
public:
	EventDispatcher(Widget *baseWidget) : m_baseWidget(baseWidget) {}

	bool Dispatch(const Event &event);
	bool DispatchSDLEvent(const SDL_Event &event);

private:
	Widget *m_baseWidget;
};

}

#endif
