#ifndef _UI_EVENTDISPATCHER_H
#define _UI_EVENTDISPATCHER_H

#include "Event.h"
#include <map>

namespace UI {

class Widget;
class Container;

class EventDispatcher {
public:
	EventDispatcher(Container *baseContainer) : m_baseContainer(baseContainer), m_mouseActiveReceiver(0), m_lastMouseOverTarget(0) {}

	bool Dispatch(const Event &event);
	bool DispatchSDLEvent(const SDL_Event &event);

	void WidgetAdded(Widget *widget);
	void WidgetRemoved(Widget *widget);

	void AddShortcut(const KeySym &keysym, Widget *target);
	void RemoveShortcut(const KeySym &keysym);
	void ClearShortcuts();

private:
	void DispatchMouseOverOut(Widget *target, const vector2f &mousePos);

	Container *m_baseContainer;
	Widget *m_mouseActiveReceiver;;
	Widget *m_lastMouseOverTarget;

	typedef std::map<KeySym,Widget*> ShortcutMap;
	ShortcutMap m_shortcuts;
};

}

#endif
