// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef UI_CONTAINER_H
#define UI_CONTAINER_H

#include "Widget.h"
#include "IterationProxy.h"
#include <vector>

// Container is the base class for all UI containers. Containers must
// provide a Layout() method that implements its layout strategy. Layout()
// will typically call PreferredSize() on its children to request their
// desired sizings then call SetSize() on its children to set their sizes
// appropriately. Containers should then call LayoutChildren() to make its
// children do their layout.
//
// Containers don't have provide Update() or Draw(). If they do they should
// make sure that they call the baseclass methods so that child widgets will
// also receive these methods.

namespace UI {

class Container: public Widget {

protected:
	// can't instantiate a base container directly
	Container(Context *context) : Widget(context), m_needsLayout(false) {}

public:
	virtual ~Container();

	virtual void Layout() = 0;
	virtual void Update();
	virtual void Draw();

	virtual bool IsContainer() const { return true; }

	// widget at pos relative to this widget
	virtual Widget *GetWidgetAt(const Point &pos);

	virtual void Disable();
	virtual void Enable();

	unsigned GetNumWidgets() const { return m_widgets.size(); }
	IterationProxy<std::vector<RefCountedPtr<Widget> > > GetWidgets() { return MakeIterationProxy(m_widgets); }
	const IterationProxy<const std::vector<RefCountedPtr<Widget> > > GetWidgets() const { return MakeIterationProxy(m_widgets); }

protected:
	void LayoutChildren();

	void AddWidget(Widget *);
	virtual void RemoveWidget(Widget *);
	void RemoveAllWidgets();

	void SetWidgetDimensions(Widget *widget, const Point &position, const Point &size);

private:

	// EventDispatcher will call here on layout change to get the shortcuts
	// for the children of this container
	friend class EventDispatcher;
	void CollectShortcuts(std::map<KeySym,Widget*> &shortcuts);

	virtual void NotifyVisible(bool visible);

	void EnableChildren();
	void DisableChildren();

	bool m_needsLayout;
	std::vector< RefCountedPtr<Widget> > m_widgets;
};

}

#endif
