// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_EVENTDISPATCHER_H
#define UI_EVENTDISPATCHER_H

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

	void LayoutUpdated();

	void AddShortcut(const KeySym &keysym, Widget *target);
	void RemoveShortcut(const KeySym &keysym);
	void ClearShortcuts();

private:
	void DispatchMouseOverOut(Widget *target, const Point &mousePos);

	Container *m_baseContainer;

	RefCountedPtr<Widget> m_mouseActiveReceiver;
	MouseButtonEvent::ButtonType m_mouseActiveTrigger;

	RefCountedPtr<Widget> m_lastMouseOverTarget;
	Point m_lastMousePosition;

	typedef std::map<KeySym,Widget*> ShortcutMap;
	ShortcutMap m_shortcuts;
};

}

#endif
