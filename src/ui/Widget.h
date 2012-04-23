#ifndef _UI_WIDGET_H
#define _UI_WIDGET_H

#include "libs.h"
#include "vector2.h"
#include "Event.h"

// Widget is the base class for all UI elements. There's a couple of things it
// must implement, and a few more it might want to implement if it wants to do
// something fancy.
//
// At minimum, a widget must implement PreferredSize() and Draw().
//

// - PreferredSize() returns a vector2 that tells the layout manager the ideal
//   size the widget would like to receive to do its job. The container is
//   under absolutely no obligation to allocate that amount of space to the
//   widget, and the widget should be prepared for that. In the worst case the
//   widget will not handle this and will draw something that either doesn't
//   use enough of its allocated space or uses too much and gets clipped.
//
// - Draw() actually draws the widget, using regular renderer calls. The
//   renderer state will be set such that the widget's top-left corner is at
//   [0,0] with a scissor region to prevent drawing outside of the widget's
//   allocated space.
//
// More advanced widgets can implement Layout() and Draw() to do more advanced
// things.
//
// - Layout() is called immediately after a widget receives its final size. It
//   will usually only be implemented by container widgets or widgets that
//   don't intend to use the entire area they've been allocated. Its job is to
//   ask its children for the preffered size, position and size them according
//   to its layout strategy, and then get them to lay out their children. See
//   Container.h for more information about implementing a container. For
//   widgets that aren't containers but don't intend to use their entire
//   allocation, they should implement Layout() and call SetActiveArea() from
//   within it.
//
// - Update() will be called after Layout() but before Draw(). The widget may
//   get its allocated size by calling GetSize(), and can do any preparation
//   for the actual draw at this point. Update() will always be called even if
//   if the widget is disabled.
//
// The GUI mainloop runs in the following order:
//
//  - input handlers
//  - Layout() (calls PreferredSize())
//  - Update()
//  - Draw()
//
// Container widgets will call Layout()/Update()/Draw() for their children.
// Thus, the GUI is a hierarchy of widgets.
//
// Input is handled via the various signals in the Widget class. A widget must
// connect to any input signals that it is interested in. External users may
// also connect to signals so they can do things (eg register button click
// handlers).
//
// Input handlers are called before Layout(), which gives a widget an
// opportunity to modify its metrics based on input. Handlers return a bool to
// indicate if the event was "handled" or not.
//
// Input handlers are called against the "leaf" widgets first. If that widget
// has no handlers for the event, or they all return false, then handlers for
// the widget's parent widget are called, and so on until either a handler
// returns true or the root widget is reached. Note that no guarantees are
// made about the order that multiple handlers attached to a single widget
// event will be called, so attaching more than one handler to an individual
// widget event is highly discouraged.

namespace UI {

class Context;
class Container;
class Metrics;
	
class Widget {
protected:
	// can't instantiate a base widget directly
	Widget(Context *context);

public:
	virtual ~Widget();

	virtual vector2f PreferredSize() = 0;
	virtual void Layout() {}
	virtual void Update() {}
	virtual void Draw() = 0;

	// gui context
	Context *GetContext() const { return m_context; }

	// enclosing container
	Container *GetContainer() const { return m_container; }

	// size allocated to widget by container
	const vector2f &GetSize() const { return m_size; }

	// position relative to container
	const vector2f &GetPosition() const { return m_position; }

	// position relative to top container
	vector2f GetAbsolutePosition() const;

	// active area of the widget. the widget may only want to use part of its
	// allocated space. drawing will be clipped to the active area, and events
	// that fall outside of the active area will be ignored. if a widget
	// doesn't set its active area it defaults to its allocated space.
	const vector2f &GetActiveArea() const { return m_activeArea; }

	// determine if a point is inside a widget's bounds
	bool Contains(const vector2f &point) const {
		return (point.x >= m_position.x && point.y >= m_position.y && point.x < m_position.x+m_activeArea.x && point.y < m_position.y+m_activeArea.y);
	}

	// deterine if an absolute point is inside a widget's bounds
	bool ContainsAbsolute(const vector2f &point) const {
		vector2f pos = GetAbsolutePosition();
		return (point.x >= pos.x && point.y >= pos.y && point.x < pos.x+m_activeArea.x && point.y < pos.y+m_activeArea.y);
	}

	// fast way to determine if the widget is a container
	virtual bool IsContainer() const { return false; }


protected:
	// this sigc accumulator calls all the handlers for an event. if any of
	// them return true, it returns true (indicating the event was handled),
	// otherwise it returns false
	//
	// declared protected so that widget subclasses can make their own
	// event signals
	struct EventHandlerResultAccumulator {
		typedef bool result_type;
		template <typename T>
		result_type operator()(T first, T last) const {
			bool result = false;
			for (; first != last; ++first)
				if (*first) result = true;
			return result;
		}
	};

public:
	
	// raw key events
	sigc::signal<bool,const KeyboardEvent &>::accumulated<EventHandlerResultAccumulator> onKeyDown;
	sigc::signal<bool,const KeyboardEvent &>::accumulated<EventHandlerResultAccumulator> onKeyUp;

	// synthesised for non-control keys. repeats when key is held down
	sigc::signal<bool,const KeyboardEvent &>::accumulated<EventHandlerResultAccumulator> onKeyPress;

	// mouse button presses
	sigc::signal<bool,const MouseButtonEvent &>::accumulated<EventHandlerResultAccumulator> onMouseDown;
	sigc::signal<bool,const MouseButtonEvent &>::accumulated<EventHandlerResultAccumulator> onMouseUp;

	// mouse movement
	sigc::signal<bool,const MouseMotionEvent &>::accumulated<EventHandlerResultAccumulator> onMouseMove;

	// mouse wheel moving
	sigc::signal<bool,const MouseWheelEvent &>::accumulated<EventHandlerResultAccumulator> onMouseWheel;

	// mouse entering or exiting widget area
	sigc::signal<bool>::accumulated<EventHandlerResultAccumulator> onMouseOver;
	sigc::signal<bool>::accumulated<EventHandlerResultAccumulator> onMouseOut;

	// click - primary mouse button press/release over widget. also
	// synthesised when keyboard shortcut is used
	sigc::signal<bool>::accumulated<EventHandlerResultAccumulator> onClick;


protected:

	// set the active area. defaults to the size allocated by the container
	void SetActiveArea(const vector2f &activeArea) { m_activeArea = activeArea; }

	// EventDispatcher needs to give us events
	friend class EventDispatcher;

	// event handlers. if emit is true the corresponding signal will be
	// fired and the value of emit will be updated with its return value. then
	// our container's Handle* method will be called with that emit value.
	// what this means in practice is that Handle* will be called for every
	// widget here to the root, whereas signals will only be emitted as long
	// as the signals continue to return false (unhandled). as such, if you
	// need to respond to an event inside a widget such that it is never
	// blocked, override the Handle* method instead of attaching to the signal
	virtual bool HandleKeyDown(const KeyboardEvent &event, bool emit = true);
	virtual bool HandleKeyUp(const KeyboardEvent &event, bool emit = true);
	virtual bool HandleMouseDown(const MouseButtonEvent &event, bool emit = true);
	virtual bool HandleMouseUp(const MouseButtonEvent &event, bool emit = true);
	virtual bool HandleMouseMove(const MouseMotionEvent &event, bool emit = true);
	virtual bool HandleMouseWheel(const MouseWheelEvent &event, bool emit = true);

	virtual bool HandleMouseOver(bool emit = true);
	virtual bool HandleMouseOut(bool emit = true);

	virtual bool HandleClick(bool emit = true);


	// mouse active. if a widget is mouse-active, it receives all mouse events
	// regardless of mouse position
	bool IsMouseActive() const { return m_mouseActive; }

	// internal synthesized events to indicate that a widget is being mouse
	// activated or deactivated. very much like MouseDown/MouseUp except you
	// get a guarantee that you will get a MouseDeactivate() call for every
	// MouseActivate(). mouse clicks trigger this
	virtual void MouseActivate();
	virtual void MouseDeactivate();

private:

	// let container set our attributes. none of them make any sense if
	// we're not in a container
	friend class Container;

	// things for the container to call to attach, detach and position the
	// widget. it could modify our data directly but that's ugly
	void Attach(Container *container);
	void Detach();
	void SetDimensions(const vector2f &position, const vector2f &size);

	// Context is the top-level container and needs to set its own context
	// and size directly
	friend class Context;
	void SetSize(const vector2f &size) { m_size = size; SetActiveArea(size); }

	Context *m_context;
	Container *m_container;
	vector2f m_position;
	vector2f m_size;
	vector2f m_activeArea;
	bool m_mouseActive;
};


// Widget set. This class is a convenience to help add multiple widgets to a
// container in a single call
class WidgetSet {
public:
	inline WidgetSet(Widget *w0) : numWidgets(1) {
		widgets[0] = w0;
	}
	inline WidgetSet(Widget *w0, Widget *w1) : numWidgets(2) {
		widgets[0] = w0; widgets[1] = w1;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2) : numWidgets(3) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3) : numWidgets(4) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4) : numWidgets(5) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4, Widget *w5) : numWidgets(6) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4; widgets[5] = w5;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4, Widget *w5, Widget *w6) : numWidgets(7) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4; widgets[5] = w5; widgets[6] = w6;
	}
	inline WidgetSet(Widget *w0, Widget *w1, Widget *w2, Widget *w3, Widget *w4, Widget *w5, Widget *w6, Widget *w7) : numWidgets(8) {
		widgets[0] = w0; widgets[1] = w1; widgets[2] = w2; widgets[3] = w3; widgets[4] = w4; widgets[5] = w5; widgets[6] = w6; widgets[7] = w7;
	}

	int numWidgets;
	Widget *widgets[8];
};

}

#endif
