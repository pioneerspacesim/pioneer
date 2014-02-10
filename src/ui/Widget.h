// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include "libs.h"
#include "Point.h"
#include "Event.h"
#include "RefCounted.h"
#include "WidgetSet.h"
#include "PropertiedObject.h"
#include <climits>
#include <set>

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
// - PreferredSize() returns a Point that tells the layout manager the ideal
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
// signals that it is interested in. External users may also connect to
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
class Layer;

class Widget : public RefCounted {
protected:
	// can't instantiate a base widget directly
	Widget(Context *context);

public:
	virtual ~Widget();

	virtual Point PreferredSize() { return Point(); }
	virtual void Layout() {}
	virtual void Update() {}
	virtual void Draw() = 0;

	// gui context
	Context *GetContext() const { return m_context; }

	// enclosing container
	Container *GetContainer() const { return m_container; }

	// size allocated to widget by container
	const Point &GetSize() const { return m_size; }

	// position relative to container
	const Point &GetPosition() const { return m_position; }

	// position relative to top container
	Point GetAbsolutePosition() const;

	// size control flags let a widget tell its container how it wants to be
	// sized when it can't get its preferred size
	Uint32 GetSizeControlFlags() const { return m_sizeControlFlags; }
	enum SizeControl { // <enum scope='UI::Widget' name=UISizeControl public>
		NO_WIDTH        = 0x01, // do not contribute preferred width to the layout
		NO_HEIGHT       = 0x02, // do not contribute preferred height to the layout
		EXPAND_WIDTH    = 0x04, // ignore preferred width, give me as much as possible
		EXPAND_HEIGHT   = 0x08, // ignore preferred height, give me as much as possible
		PRESERVE_ASPECT = 0x10, // allocate same aspect ratio as preferred size
	};

	// draw offset. used to move a widget "under" its visible area (scissor)
	void SetDrawOffset(const Point &drawOffset) { m_drawOffset = drawOffset; }
	const Point &GetDrawOffset() const { return m_drawOffset; }

	// active area of the widget. the widget may only want to use part of its
	// allocated space. drawing will be clipped to the active area, and events
	// that fall outside of the active area will be ignored. if a widget
	// doesn't set its active area it defaults to its allocated space.
	const Point &GetActiveOffset() const { return m_activeOffset; }
	const Point &GetActiveArea() const { return m_activeArea; }

	// determine if a point is inside a widgets active area
	bool Contains(const Point &point) const {
		const Point min_corner = (m_activeOffset - m_drawOffset);
		const Point max_corner = (min_corner + m_activeArea);
		return (point.x >= min_corner.x && point.y >= min_corner.y && point.x < max_corner.x && point.y < max_corner.y);
	}

	// calculate layout contribution based on preferred size and flags
	Point CalcLayoutContribution();
	// calculate size based on available space, preferred size and flags
	Point CalcSize(const Point &avail);

	// fast way to determine if the widget is a container
	virtual bool IsContainer() const { return false; }

	// selectable widgets may receive keyboard focus
	virtual bool IsSelectable() const { return false; }

	// disabled widgets do not receive input
	virtual void Disable();
	virtual void Enable();
	bool IsDisabled() const { return m_disabled; }

	bool IsMouseOver() const { return m_mouseOver; }

	// register a key that, when pressed and not handled by any other widget,
	// will cause a click event to be sent to this widget
	void AddShortcut(const KeySym &keysym) { m_shortcuts.insert(keysym); }
	void RemoveShortcut(const KeySym &keysym) { m_shortcuts.erase(keysym); }

	// font size. obviously used for text size but also sometimes used for
	// general widget size (eg space size). might do nothing, depends on the
	// widget
	enum Font { // <enum scope='UI::Widget' name=UIFont prefix=FONT_ public>
		FONT_XSMALL,
		FONT_SMALL,
		FONT_NORMAL,
		FONT_LARGE,
		FONT_XLARGE,

		FONT_HEADING_XSMALL,
		FONT_HEADING_SMALL,
		FONT_HEADING_NORMAL,
		FONT_HEADING_LARGE,
		FONT_HEADING_XLARGE,

		FONT_MONO_XSMALL,
		FONT_MONO_SMALL,
		FONT_MONO_NORMAL,
		FONT_MONO_LARGE,
		FONT_MONO_XLARGE,

		FONT_MAX,                 // <enum skip>

		FONT_INHERIT,

		FONT_SMALLEST         = FONT_XSMALL,         // <enum skip>
		FONT_LARGEST          = FONT_XLARGE,         // <enum skip>
		FONT_HEADING_SMALLEST = FONT_HEADING_XSMALL, // <enum skip>
		FONT_HEADING_LARGEST  = FONT_HEADING_XLARGE, // <enum skip>
		FONT_MONO_SMALLEST    = FONT_MONO_XSMALL,    // <enum skip>
		FONT_MONO_LARGEST     = FONT_MONO_XLARGE,    // <enum skip>
	};

	virtual Widget *SetFont(Font font);
	Font GetFont() const;

	// widget id. used for queries/searches
	const std::string &GetId() const { return m_id; }
	Widget *SetId(const std::string &id) { m_id = id; return this; }

	// bind an object property to a widget bind point
	void Bind(const std::string &bindName, PropertiedObject *object, const std::string &propertyName);

	// this sigc accumulator calls all the handlers for an event. if any of
	// them return true, it returns true (indicating the event was handled),
	// otherwise it returns false
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


	// raw key events
	sigc::signal<bool,const KeyboardEvent &>::accumulated<EventHandlerResultAccumulator> onKeyDown;
	sigc::signal<bool,const KeyboardEvent &>::accumulated<EventHandlerResultAccumulator> onKeyUp;

	// text input, full unicode codepoint
	sigc::signal<bool,const TextInputEvent &>::accumulated<EventHandlerResultAccumulator> onTextInput;

	// mouse button presses
	sigc::signal<bool,const MouseButtonEvent &>::accumulated<EventHandlerResultAccumulator> onMouseDown;
	sigc::signal<bool,const MouseButtonEvent &>::accumulated<EventHandlerResultAccumulator> onMouseUp;

	// mouse movement
	sigc::signal<bool,const MouseMotionEvent &>::accumulated<EventHandlerResultAccumulator> onMouseMove;

	// mouse wheel moving
	sigc::signal<bool,const MouseWheelEvent &>::accumulated<EventHandlerResultAccumulator> onMouseWheel;

	// joystick events
	sigc::signal<bool,const JoystickAxisMotionEvent &>::accumulated<EventHandlerResultAccumulator> onJoystickAxisMove;
	sigc::signal<bool,const JoystickHatMotionEvent &>::accumulated<EventHandlerResultAccumulator> onJoystickHatMove;
	sigc::signal<bool,const JoystickButtonEvent &>::accumulated<EventHandlerResultAccumulator> onJoystickButtonDown;
	sigc::signal<bool,const JoystickButtonEvent &>::accumulated<EventHandlerResultAccumulator> onJoystickButtonUp;

	// mouse entering or exiting widget area
	sigc::signal<bool>::accumulated<EventHandlerResultAccumulator> onMouseOver;
	sigc::signal<bool>::accumulated<EventHandlerResultAccumulator> onMouseOut;

	// click - primary mouse button press/release over widget. also
	// synthesised when keyboard shortcut is used
	sigc::signal<bool>::accumulated<EventHandlerResultAccumulator> onClick;


protected:

	// magic constant for PreferredSize to indicate "as much as possible"
	static const int SIZE_EXPAND = INT_MAX;

	// safely add two sizes, preserving SIZE_EXPAND
	static inline int SizeAdd(int a, int b) { return a == SIZE_EXPAND || b == SIZE_EXPAND ? SIZE_EXPAND : a+b; }
	static inline Point SizeAdd(const Point &a, const Point &b) { return Point(SizeAdd(a.x,b.x), SizeAdd(a.y,b.y)); }

	// set size control flags. no flags by default
	void SetSizeControlFlags(Uint32 flags) { m_sizeControlFlags = flags; }

	// set the active area. defaults to the size allocated by the container
	void SetActiveArea(const Point &activeArea, const Point &activeOffset = Point());

	// mouse active. if a widget is mouse-active, it receives all mouse events
	// regardless of mouse position
	bool IsMouseActive() const;

	bool IsSelected() const;

	Point GetMousePos() const;

	// indicates whether the widget is part of the visible tree of widgets
	// (ie, its chain of parents links to a Context)
	bool IsVisible() const { return m_visible; }

	void SetDisabled(bool disabled) { m_disabled = disabled; }

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
	virtual void HandleJoystickAxisMove(const JoystickAxisMotionEvent &event) {}
	virtual void HandleJoystickHatMove(const JoystickHatMotionEvent &event) {}
	virtual void HandleJoystickButtonDown(const JoystickButtonEvent &event) {}
	virtual void HandleJoystickButtonUp(const JoystickButtonEvent &event) {}

	virtual void HandleClick() {}

	virtual void HandleMouseOver() {}
	virtual void HandleMouseOut() {}

	// internal synthesized events to indicate that a widget is being mouse
	// activated or deactivated. very much like MouseDown/MouseUp except you
	// get a guarantee that you will get a MouseDeactivate() call for every
	// MouseActivate(). mouse clicks trigger this
	virtual void HandleMouseActivate() {}
	virtual void HandleMouseDeactivate() {}

	// text input event, a full unicode codepoint
	virtual void HandleTextInput(const TextInputEvent &event) {}

	// internal synthesized events fired when a widget is selected or
	// deselected. on mousedown, a widget becomes the selected widget unless
	// its IsSelectable method returns false. the previously-selected widget
	// (if there was one) gets deselected
	virtual void HandleSelect() {}
	virtual void HandleDeselect() {}

	virtual void HandleVisible() {}
	virtual void HandleInvisible() {}

	void RegisterBindPoint(const std::string &bindName, sigc::slot<void,PropertyMap &,const std::string &> method);

private:

	// EventDispatcher needs to give us events
	friend class EventDispatcher;

	// event triggers. when called:
	//  - calls the corresponding Handle* method on this widget (always)
	//  - fires the corresponding on* signal on this widget (iff handled is false)
	//  - calls the container Trigger* method with the new handled value returned
	//    by the on* signal
	//
	// what this means in practic is that Handle* will be called for every
	// widget from here to the root, whereas signals will only be fired as
	// long as the signals continue to return false (unhandled).
	bool TriggerKeyDown(const KeyboardEvent &event, bool handled = false);
	bool TriggerKeyUp(const KeyboardEvent &event, bool handled = false);
	bool TriggerTextInput(const TextInputEvent &event, bool handled = false);
	bool TriggerMouseDown(const MouseButtonEvent &event, bool handled = false);
	bool TriggerMouseUp(const MouseButtonEvent &event, bool handled = false);
	bool TriggerMouseMove(const MouseMotionEvent &event, bool handled = false);
	bool TriggerMouseWheel(const MouseWheelEvent &event, bool handled = false);

	bool TriggerJoystickButtonDown(const JoystickButtonEvent &event, bool handled = false);
	bool TriggerJoystickButtonUp(const JoystickButtonEvent &event, bool handled = false);
	bool TriggerJoystickAxisMove(const JoystickAxisMotionEvent &event, bool handled = false);
	bool TriggerJoystickHatMove(const JoystickHatMotionEvent &event, bool handled = false);

	bool TriggerClick(bool handled = false);

	// stop is used during disable/enable to stop delivery at the given widget
	bool TriggerMouseOver(const Point &pos, bool handled = false, Widget *stop = 0);
	bool TriggerMouseOut(const Point &pos, bool handled = false, Widget *stop = 0);

	void TriggerMouseActivate();
	void TriggerMouseDeactivate();

	void TriggerSelect();
	void TriggerDeselect();


	// let container set our attributes. none of them make any sense if
	// we're not in a container
	friend class Container;

	// things for the container to call to attach, detach and position the
	// widget. it could modify our data directly but that's ugly
	void Attach(Container *container);
	void Detach();
	void SetDimensions(const Point &position, const Point &size);
	virtual void NotifyVisible(bool visible);

	// called by Container::CollectShortcuts
	const std::set<KeySym> &GetShortcuts() const { return m_shortcuts; }


	// Context is the top-level container and needs to set its own context
	// and size directly
	friend class Context;
	void SetSize(const Point &size) { m_size = size; SetActiveArea(size); }

	Context *m_context;
	Container *m_container;

	Point m_position;
	Point m_size;

	Uint32 m_sizeControlFlags;

	Point m_drawOffset;

	Point m_activeOffset;
	Point m_activeArea;

	Font m_font;

	bool m_disabled;

	bool m_mouseOver;
	bool m_visible;

	std::set<KeySym> m_shortcuts;

	std::string m_id;

	std::map< std::string,sigc::slot<void,PropertyMap &,const std::string &> > m_bindPoints;
	std::map< std::string,sigc::connection > m_binds;
};

}

#endif
