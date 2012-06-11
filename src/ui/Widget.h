#ifndef _UI_WIDGET_H
#define _UI_WIDGET_H

#include "libs.h"
#include "vector2.h"
#include "Event.h"
#include "RefCounted.h"

// Widget is the base class for all UI elements. There's a couple of things it
// must implement, and a few more it might want to implement if it wants to do
// something fancy.
//
// At minimum, a widget must implement Draw().
//
// - Draw() actually draws the widget, using regular renderer calls. The
//   renderer state will be set such that the widget's top-left corner is at
//   [0,0] with a scissor region to prevent drawing outside of the widget's
//   allocated space.
//
// Widgets can implement PreferredSize(), Layout() and Draw() to do more
// advanced things.
//
// - PreferredSize() returns a vector2 that tells the layout manager the ideal
//   size the widget would like to receive to do its job. The container is
//   under absolutely no obligation to allocate that amount of space to the
//   widget, and the widget should be prepared for that. In the worst case the
//   widget will not handle this and will draw something that either doesn't
//   use enough of its allocated space or uses too much and gets clipped.
//
// - Layout() is called to tell a container widget (or a widget that does not
//   intend to use its entire allocated area) to ask its children for their
//   preferred size, position and size them according to its layout strategy,
//   and then ask them to lay out their children. As such, when Layout is called
//   a widget can assume that its size and position have been set. It is only
//   called occassionally when the layout becomes invalid, typically after a
//   widget is added or removed. See Container.h for more information about
//   implementing a container. Widgets that aren't containers but don't intend
//   to use their entire allocation should implement Layout() and call
//   SetActiveArea() from it.
//   
// - Update() is called every frame before Draw(). The widget may get its
//   allocated size by calling GetSize() and can do any preparation for the
//   actual draw at this point.
//
// Each frame a widget will receive calls in the following order:
//
//  - event handlers (user input)
//  - Layout() (if layout needs recalculating)
//  - event handlers (from layout changes)
//  - Update()
//  - Draw()
//
// Input is fed into the context's event dispatcher and used to generate the
// various events in the Widget class. A widget must connect to any event
// singlas that it is interested in. External users may also connect to
// signals to do things (eg register button click handlers).
//
// Event handlers for events generated from user input are called before
// Layout(). Layout() may also generate events (such as mouse over/out events)
// as widgets move.
//
// Event handlers from user input are called before Layout(), which gives a
// widget an opportunity to modify the layout based on input. If a widget
// wants to change its size it must call GetContext()->RequestLayout() to
// force a layout change to occur.
//
// opportunity to modify its metrics based on input. Handlers return a bool to
// indicate if the event was "handled" or not.
//
// Event handlers are called against the "leaf" widgets first. Handlers return
// a bool to indicate if the event was "handled" or not. If a widget has no
// handlers for the event, or they all return false, then handlers for the
// widget's container are called, and so on until either a handler returns
// true or the root widget (context) is reached. Note that no guarantees are
// made about the order that multiple handlers attached to a single widget
// event will be called, so attaching more than one handler to an individual
// widget event is highly discouraged.

namespace UI {

class Context;
class Container;
class Metrics;
	
class Widget : public RefCounted {
protected:
	// can't instantiate a base widget directly
	Widget(Context *context);

public:
	virtual ~Widget();

	virtual vector2f PreferredSize() { return 0; }
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
	const vector2f &GetActiveOffset() const { return m_activeOffset; }
	const vector2f &GetActiveArea() const { return m_activeArea; }

	// determine if a point is inside a widget's bounds
	bool Contains(const vector2f &point) const {
		return (point.x >= m_activeOffset.x && point.y >= m_activeOffset.y && point.x < m_activeOffset.x+m_activeArea.x && point.y < m_activeOffset.y+m_activeArea.y);
	}

	// deterine if an absolute point is inside a widget's bounds
	bool ContainsAbsolute(const vector2f &point) const {
		vector2f pos = GetAbsolutePosition() + m_activeOffset;
		return (point.x >= pos.x && point.y >= pos.y && point.x < pos.x+m_activeArea.x && point.y < pos.y+m_activeArea.y);
	}

	// fast way to determine if the widget is a container
	virtual bool IsContainer() const { return false; }

	// are we floating
	bool IsFloating() const { return m_floating; }

	// set/get the current draw transform. useful for animations, special effects
	void SetTransform(const matrix4x4f &transform) { m_transform = transform; }
	const matrix4x4f &GetTransform() const { return m_transform; }

	// font size. obviously used for text size but also sometimes used for
	// general widget size (eg space size). might do nothing, depends on the
	// widget
	enum FontSize {
		FONT_SIZE_XSMALL,
		FONT_SIZE_SMALL,
		FONT_SIZE_NORMAL,
		FONT_SIZE_LARGE,
		FONT_SIZE_XLARGE,
		FONT_SIZE_MAX
	};

	virtual Widget *SetFontSize(FontSize fontSize);
	FontSize GetFontSize() const { return m_fontSize; }


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
	void SetActiveArea(const vector2f &activeArea, const vector2f &activeOffset = 0);

	// mouse active. if a widget is mouse-active, it receives all mouse events
	// regardless of mouse position
	bool IsMouseActive() const { return m_mouseActive; }

	// internal event handlers. override to handle events. unlike the external
	// on* signals, every widget in the stack is guaranteed to receive a call
	// - there's no facility for stopping propogation up the stack
    //
	// as such, if you need to respond to an event inside a widget always
    // without worrying about it being blocked, you should override the
    // Handle* method instead of attaching to the signal.
	virtual void HandleKeyDown(const KeyboardEvent &event) {}
	virtual void HandleKeyUp(const KeyboardEvent &event) {}
	virtual void HandleMouseDown(const MouseButtonEvent &event) {}
	virtual void HandleMouseUp(const MouseButtonEvent &event) {}
	virtual void HandleMouseMove(const MouseMotionEvent &event) {}
	virtual void HandleMouseWheel(const MouseWheelEvent &event) {}

	virtual void HandleClick() {}

	virtual void HandleMouseOver() {}
	virtual void HandleMouseOut() {}

	// internal synthesized events to indicate that a widget is being mouse
	// activated or deactivated. very much like MouseDown/MouseUp except you
	// get a guarantee that you will get a MouseDeactivate() call for every
	// MouseActivate(). mouse clicks trigger this
	virtual void HandleMouseActivate() {}
	virtual void HandleMouseDeactivate() {}


private:

	// EventDispatcher needs to give us events
	friend class EventDispatcher;

	// event triggers. when called:
	//  - calls the corresponding Handle* method on this widget (always)
	//  - fires the corresponding on* signal on this widget (iff emit is true)
	//  - calls the container Trigger* method with the new emit value returned
	//    by the on* signal
	//
	// what this means in practic is that Handle* will be called for every
	// widget from here to the root, whereas signals will only be fired as
	// long as the signals continue to return false (unhandled).
	bool TriggerKeyDown(const KeyboardEvent &event, bool emit = true);
	bool TriggerKeyUp(const KeyboardEvent &event, bool emit = true);
	bool TriggerMouseDown(const MouseButtonEvent &event, bool emit = true);
	bool TriggerMouseUp(const MouseButtonEvent &event, bool emit = true);
	bool TriggerMouseMove(const MouseMotionEvent &event, bool emit = true);
	bool TriggerMouseWheel(const MouseWheelEvent &event, bool emit = true);

	bool TriggerClick(bool emit = true);

	bool TriggerMouseOver(const vector2f &pos, bool emit = true);
	bool TriggerMouseOut(const vector2f &pos, bool emit = true);

	void TriggerMouseActivate();
	void TriggerMouseDeactivate();


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

	// FloatContainer needs to change floating state
	friend class FloatContainer;
	void SetFloating(bool floating) { m_floating = floating; }

	Context *m_context;
	Container *m_container;
	vector2f m_position;
	vector2f m_size;

	vector2f m_activeOffset;
	vector2f m_activeArea;

	matrix4x4f m_transform;
	FontSize m_fontSize;

	bool m_floating;

	bool m_mouseOver;
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
