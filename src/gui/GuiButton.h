// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIBUTTON_H
#define _GUIBUTTON_H

#include "GuiWidget.h"
#include <string>

namespace Gui {
	class Label;

	class Button: public Widget {
	public:
		Button();
		virtual ~Button();
		virtual bool OnMouseDown(MouseButtonEvent *e);
		virtual bool OnMouseUp(MouseButtonEvent *e);
		virtual void OnActivate();

		// onClick only happens when press and release are both on widget (release can be elsewhere)
		sigc::signal<void> onPress;
		sigc::signal<void> onRelease;
		sigc::signal<void> onClick;
		bool IsPressed() { return m_isPressed; }
	private:
		void OnRawMouseUp(MouseButtonEvent *e);
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

	class LabelButton: public Button {
	public:
		LabelButton(Label *label);
		virtual ~LabelButton();
		virtual void GetSizeRequested(float size[2]);
		virtual void Draw();
		void SetPadding(float p) { m_padding = p; }
	protected:
		Label *m_label;
	private:
		void OnSetSize();
		float m_padding;
	};
}

#endif /* _GUIBUTTON_H */
