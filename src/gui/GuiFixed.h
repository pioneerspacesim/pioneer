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
		Fixed();
		void Add(Widget *child, float x, float y);
		void Remove(Widget *child);
		virtual ~Fixed();
		virtual void GetSizeRequested(float size[2]);
		virtual void OnChildResizeRequest(Widget *);
		virtual void UpdateAllChildSizes();
		void SetSizeRequest(float x, float y);
		void SetSizeRequest(float size[2]);
	private:
		void _Init();
		float m_userWantedSize[2];
	};
}

#endif /* _GUIFIXED_H */

