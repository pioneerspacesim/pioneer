#ifndef _UI_EVENTDISPATCHER_H
#define _UI_EVENTDISPATCHER_H

#include "Event.h"

namespace UI {

class Widget;
class Container;

class EventDispatcher {
public:
	EventDispatcher(Container *baseContainer) : m_baseContainer(baseContainer), m_mouseDownReceiver(0), m_mouseMoveReceiver(0) {}

	bool Dispatch(const Event &event);
	bool DispatchSDLEvent(const SDL_Event &event);

private:
	Container *m_baseContainer;
	Widget *m_mouseDownReceiver;
	Widget *m_mouseMoveReceiver;
};

}

#endif
