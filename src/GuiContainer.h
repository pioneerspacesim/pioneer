#ifndef _GUICONTAINER_H
#define _GUICONTAINER_H
/*
 * Parent of all widgets that contain other widgets.
 */

#include "GuiWidget.h"
#include <list>

namespace Gui {
	class Container: public Widget {
	public:
		bool OnMouseDown(MouseButtonEvent *e);
		bool OnMouseUp(MouseButtonEvent *e);
		void DeleteAllChildren();
		virtual void Draw();
		virtual void ShowAll();
		virtual void HideAll();
	private:
		bool HandleMouseEvent(MouseButtonEvent *e);
	protected:
		void PrependChild(Widget *w, float x, float y);
		void AppendChild(Widget *w, float x, float y);

		struct widget_pos {
			Widget *w;
			float pos[2];
		};
		std::list<widget_pos> m_children;
	};
}

#endif /* _GUICONTAINER_H */

