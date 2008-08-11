#ifndef _GUIWIDGET_H
#define _GUIWIDGET_H

#include "GuiEvents.h"


namespace Gui {
	class Container;
	class Widget {
	public:
		Widget();
		virtual void Draw() = 0;
		virtual ~Widget() {}
		virtual void GetSizeRequested(float size[2]) = 0;
		void GetPosition(float pos[2]) { pos[0] = m_size.x; pos[1] = m_size.y; }
		void SetPosition(float x, float y) { m_size.x = x; m_size.y = y; }
		void GetSize(float size[2]) { size[0] = m_size.w; size[1] = m_size.h; }
		void SetSize(float w, float h) { m_size.w = w; m_size.h = h; }
		void SetShortcut(SDLKey key, SDLMod mod);
		virtual void Show() { m_visible = true; }
		virtual void Hide() { m_visible = false; }
		bool IsVisible() { return m_visible; }
		Container *GetParent() { return m_parent; }
		void SetParent(Container *p) { m_parent = p; }

		// event handlers should return false to stop propagating event
		virtual bool OnMouseDown(MouseButtonEvent *e) { return true; }
		virtual bool OnMouseUp(MouseButtonEvent *e) { return true; }
		virtual void OnActivate() {}
		// only to be called by Screen::OnKeyDown
		void OnPreShortcut(const SDL_keysym *sym);
		enum EventMask {
			EVENT_NONE = 0,
			EVENT_KEYDOWN = 1<<0,
			EVENT_KEYUP = 1<<1,
			EVENT_MOUSEDOWN = 1<<2,
			EVENT_MOUSEUP = 1<<3,
			EVENT_ALL = 0xffffffff
		};
		unsigned int GetEventMask() { return m_eventMask; }
	protected:
		unsigned int m_eventMask;
		struct {
			SDLKey sym;
			SDLMod mod;
		} m_shortcut;
	private:
		struct {
			float x,y,w,h;
		} m_size;
		bool m_visible;
		Container *m_parent;
	};
}

#endif /* _GUIWIDGET_H */
