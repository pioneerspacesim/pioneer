// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

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
		void RemoveAllChildren();
		void DeleteAllChildren();
		void GetChildPosition(const Widget *child, float outPos[2]) const;
		int GetNumChildren() { return m_children.size(); }
		virtual void Draw();
		void ShowChildren();
		void HideChildren();
		virtual void Show();
		virtual void ShowAll();
		virtual void HideAll();
		virtual void OnChildResizeRequest(Widget *) = 0;
		void SetBgColor(const Color &col);
		void SetBgColor(float r, float g, float b, float a);
		void SetTransparency(bool a) { m_transparent = a; }
		virtual void UpdateAllChildSizes() = 0;
		void RemoveChild(Widget *w);
		// only fired if child widgets do not eat event
		sigc::signal<void, MouseButtonEvent*> onMouseButtonEvent;
	private:
		void _OnMouseLeave();
		void _OnSetSize();
		bool HandleMouseEvent(MouseButtonEvent *e);
		Color m_bgcol;
		bool m_transparent;
	protected:
		void PrependChild(Widget *w, float x, float y);
		void AppendChild(Widget *w, float x, float y);
		void MoveChild(Widget *w, float x, float y);

		struct widget_pos {
			Widget *w;
			float pos[2];
			Uint32 flags;
		};
		std::list<widget_pos> m_children;
	};
}

#endif /* _GUICONTAINER_H */

