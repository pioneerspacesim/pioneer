#ifndef _GUITOGGLEBUTTON_H
#define _GUITOGGLEBUTTON_H

#include "GuiWidget.h"
#include <string>

namespace Gui {
	class ToggleButton: public Button {
	public:
		ToggleButton();
		virtual void Draw();
		virtual ~ToggleButton() {}
		virtual void GetSizeRequested(float size[2]);
		virtual bool OnMouseDown(MouseButtonEvent *e);
		virtual void OnActivate();
		void SetPressed(bool s) { m_pressed = s; }
		bool GetPressed() { return m_pressed != 0; }

		sigc::signal<void, ToggleButton *, bool> onChange;
	private:
		bool m_pressed;
	};
}

#endif /* _GUITOGGLEBUTTON_H */
