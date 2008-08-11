#ifndef _GUIBUTTON_H
#define _GUIBUTTON_H

#include "GuiWidget.h"
#include <string>

namespace Gui {
	class Button: public Widget {
	public:
		Button();
		virtual ~Button() {}
		virtual bool OnMouseDown(MouseButtonEvent *e);
		virtual bool OnMouseUp(MouseButtonEvent *e);
		virtual void OnActivate();
		
		// onClick only happens when press and release are both on widget (release can be elsewhere)
		sigc::signal<void> onPress;
		sigc::signal<void> onRelease;
		sigc::signal<void> onClick;
		bool IsPressed() { return m_isPressed; }
	private:
		void OnRawMouseUp(SDL_MouseButtonEvent *e);
		void OnRawKeyUp(SDL_KeyboardEvent *e);

		bool m_isPressed;
		sigc::connection _m_release;
		sigc::connection _m_kbrelease;
	};

	class SolidButton: public Button {
	public:
		SolidButton(): Button() {}
		virtual ~SolidButton() {}
		virtual void GetSizeRequested(float size[2]);
		virtual void Draw();
	};

	class TransparentButton: public Button {
	public:
		TransparentButton(): Button() {}
		virtual ~TransparentButton() {}
		virtual void GetSizeRequested(float size[2]);
		virtual void Draw();
	};
}

#endif /* _GUIBUTTON_H */
