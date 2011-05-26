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

		virtual Widget *GetTopWidget();
		virtual void PushWidget(Widget *w);
		virtual void PopWidget();
		virtual void ClearWidgets();
		virtual void JumpToWidget(Widget *w);

	private:
		std::stack<Widget*> m_widgets;
	};
}

#endif
