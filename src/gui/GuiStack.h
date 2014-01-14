// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUISTACK_H
#define _GUISTACK_H

#include "GuiWidget.h"
#include "GuiContainer.h"
#include <stack>

namespace Gui {
	class Stack: public Container {
	public:
		Stack();
		virtual ~Stack();

		virtual void GetSizeRequested(float size[2]);
		virtual void OnChildResizeRequest(Widget *w);
		virtual void UpdateAllChildSizes();
		virtual void ShowAll();

		virtual Widget *Top();
		virtual int Size();
		virtual void Push(Widget *w);
		virtual void Pop();
		virtual void Clear();
		virtual void JumpTo(Widget *w);

	private:
		std::stack<Widget*> m_widgets;
	};
}

#endif
