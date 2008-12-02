#ifndef _GUIFIXED_H
#define _GUIFIXED_H
/*
 * Fixed position widget container.
 */

#include "GuiWidget.h"
#include "GuiContainer.h"

namespace Gui {
	class Fixed: public Container {
	public:
		Fixed(float w, float h);
		void Add(Widget *child, float x, float y);
		void Remove(Widget *child);
		virtual void Draw();
		virtual ~Fixed();
		virtual void GetSizeRequested(float size[2]);
		virtual void OnChildResizeRequest(Widget *);
		void SetBgColor(float rgb[3]);
		void SetBgColor(float r, float g, float b);
		void SetTransparency(bool a) { m_transparent = a; }
	private:
		float m_w, m_h;
		float m_bgcol[3];
		bool m_transparent;
	};
}

#endif /* _GUIFIXED_H */

