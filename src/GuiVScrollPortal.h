#ifndef _GUIVSCROLLPORTAL_H
#define _GUIVSCROLLPORTAL_H

#include "GuiContainer.h"

namespace Gui {
	class VScrollPortal: public Container 
	{
	public:
		VScrollPortal(float w, float h);
		void Add(Widget *child);
		void Remove(Widget *child);
		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);
		virtual void OnChildResizeRequest(Widget *);
		void SetBgColor(float rgb[3]);
		void SetBgColor(float r, float g, float b);
		Adjustment vscrollAdjust;
	private:
		void OnScroll(float);
		float m_scrollY, m_childSizeY;
		Widget *m_child;
	};
}

#endif /* _GUIVSCROLLPORTAL_H */
