// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIVSCROLLPORTAL_H
#define _GUIVSCROLLPORTAL_H

#include "GuiContainer.h"

namespace Gui {
	class VScrollPortal: public Container
	{
	public:
		VScrollPortal(float forceWidth);
		VScrollPortal();
		void Add(Widget *child);
		void Remove(Widget *child);
		virtual bool OnMouseDown(MouseButtonEvent *e);
		virtual bool OnMouseUp(MouseButtonEvent *e);
		virtual bool OnMouseMotion(MouseMotionEvent *e);
		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);
		virtual void GetMinimumSize(float size[2]);
		virtual void OnChildResizeRequest(Widget *);
		virtual void UpdateAllChildSizes();
		void SetBgColor(float rgb[3]);
		void SetBgColor(float r, float g, float b);
		Adjustment vscrollAdjust;
	private:
		float GetScrollPixels();
		void OnScroll(float);
		float m_forceWidth;
		float m_scrollY, m_childSizeY;
		Widget *m_child;
	};
}

#endif /* _GUIVSCROLLPORTAL_H */
