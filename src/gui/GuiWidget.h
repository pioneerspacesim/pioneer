#ifndef _GUIWIDGET_H
#define _GUIWIDGET_H

#include "Color.h"
#include "GuiEvents.h"

namespace Gui {
	class Container;
	class ToolTip;
	class Widget {
	public:
		Widget();
		virtual void Draw() = 0;
		virtual ~Widget();
		/** containers call this on children. input: size[] will contain max permissible size
		 *  output: size[] will contain what space the widget desires */
		virtual void GetSizeRequested(float size[2]) = 0;
		// the minimum size the widget requires to operate effectively
		virtual void GetMinimumSize(float size[2]) { GetSizeRequested(size); }
		void GetAbsolutePosition(float pos[2]) const;
		void GetSize(float size[2]) { size[0] = m_size.w; size[1] = m_size.h; }
		void SetSize(float w, float h) { m_size.w = w; m_size.h = h; onSetSize.emit(); }
		void ResizeRequest();
		void SetShortcut(SDLKey key, SDLMod mod);
		void SetClipping(float width, float height);
		void EndClipping();
		bool GetEnabled() { return m_enabled; }
		void SetEnabled(bool v) { m_enabled = v; }
		virtual void GrabFocus();
		bool IsFocused();
		virtual void ShowAll() { m_visible = true; }
		virtual void Show() { m_visible = true; }
		virtual void Hide();
		bool IsVisible() const;
		Container *GetParent() const { return m_parent; }
		void SetParent(Container *p) { m_parent = p; }
		void SetToolTip(std::string s) { m_tooltip = s; }
		const std::string &GetToolTip() const { return m_tooltip; }

		// event handlers should return false to stop propagating event
		virtual bool OnMouseDown(MouseButtonEvent *e) { return true; }
		virtual bool OnMouseUp(MouseButtonEvent *e) { return true; }
		virtual bool OnMouseMotion(MouseMotionEvent *e) { return true; }
		virtual void OnActivate() {}
		virtual void OnMouseEnter();
		virtual void OnMouseLeave();
		virtual bool OnKeyPress(const SDL_keysym *sym) { return false; }
		bool IsMouseOver() { return m_mouseOver; }
		// only to be called by Screen::OnKeyDown
		void OnPreShortcut(const SDL_keysym *sym);
		enum EventMask {
			EVENT_NONE = 0,
			EVENT_KEYDOWN = 1<<0,
			EVENT_KEYUP = 1<<1,
			EVENT_MOUSEDOWN = 1<<2,
			EVENT_MOUSEUP = 1<<3,
			EVENT_MOUSEMOTION = 1<<4, // needed for OnMouseEnter,Leave,IsMouseOver
			EVENT_ALL = 0xffffffff
		};
		unsigned int GetEventMask() { return m_eventMask; }

		sigc::signal<void> onMouseEnter;
		sigc::signal<void> onMouseLeave;
		sigc::signal<void> onSetSize;
		sigc::signal<void> onDelete;
	protected:
		unsigned int m_eventMask;
		struct {
			SDLKey sym;
			SDLMod mod;
		} m_shortcut;
		
		virtual std::string GetOverrideTooltip() { return ""; }
		void UpdateOverriddenTooltip();
	private:
		struct {
			float w,h;
		} m_size;
		bool m_visible;
		bool m_mouseOver;
		bool m_enabled;
		Container *m_parent;
		std::string m_tooltip;
		sigc::connection m_tooltipTimerConnection;
		ToolTip *m_tooltipWidget;
		void OnToolTip();
	};
}

#endif /* _GUIWIDGET_H */
