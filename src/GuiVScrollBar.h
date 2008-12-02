#ifndef _GUIVSCROLLBAR
#define _GUIVSCROLLBAR

#include "GuiWidget.h"

namespace Gui {
	class VScrollBar: public Widget {
	public:
		VScrollBar();
		virtual ~VScrollBar() {}
		virtual bool OnMouseDown(MouseButtonEvent *e);
		virtual void GetSizeRequested(float size[2]);
		virtual void Draw();
		void SetAdjustment(Adjustment *adj) {
			m_adjustment = adj;
		}
	private:
		void OnRawMouseUp(SDL_MouseButtonEvent *e);
		void OnRawMouseMotion(SDL_MouseMotionEvent *e);
		bool m_isPressed;
		sigc::connection _m_release, _m_motion;
		Adjustment *m_adjustment;
	};
}

#endif /* _GUIVSCROLLBAR */
