// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIVSCROLLBAR
#define _GUIVSCROLLBAR

#include "GuiWidget.h"

namespace Gui {
	class ScrollBar: public Widget {
	public:
		ScrollBar(bool isHoriz);
		virtual ~ScrollBar();
		virtual bool OnMouseDown(MouseButtonEvent *e);
		virtual void GetSizeRequested(float size[2]);
		virtual void GetMinimumSize(float size[2]);
		virtual void Draw();
		void SetAdjustment(Adjustment *adj) {
			m_adjustment = adj;
		}
	protected:
		Adjustment *m_adjustment;
	private:
		void OnRawMouseUp(MouseButtonEvent *e);
		void OnRawMouseMotion(MouseMotionEvent *e);
		bool m_isPressed, m_isHoriz;
		sigc::connection _m_release, _m_motion;
	};

	class VScrollBar: public ScrollBar {
	public:
		VScrollBar(): ScrollBar(false) {}
	};
}

#endif /* _GUIVSCROLLBAR */
