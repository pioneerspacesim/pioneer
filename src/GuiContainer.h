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
		Container();
		virtual ~Container();
		bool OnMouseDown(MouseButtonEvent *e);
		bool OnMouseUp(MouseButtonEvent *e);
		bool OnMouseMotion(MouseMotionEvent *e);
		void DeleteAllChildren();
		virtual void Draw();
		virtual void ShowAll();
		virtual void HideAll();
		virtual void OnChildResizeRequest(Widget *) = 0;
	private:
		void _OnMouseLeave();
		void _OnSetSize();
		bool HandleMouseEvent(MouseButtonEvent *e);
	protected:
		void PrependChild(Widget *w, float x, float y);
		void AppendChild(Widget *w, float x, float y);
		void MoveChild(Widget *w, float x, float y);
		void RemoveChild(Widget *w);

		struct widget_pos {
			Widget *w;
			float pos[2];
		};
		std::list<widget_pos> m_children;
	};
}

#endif /* _GUICONTAINER_H */

