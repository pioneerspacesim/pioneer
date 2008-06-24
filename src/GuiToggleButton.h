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
		virtual void GetSizeRequested(float &w, float &h);
		virtual void OnMouseDown(MouseButtonEvent *e);
		void SetPressed(bool s) { m_pressed = s; }
		bool GetPressed() { return m_pressed; }
		
		sigc::signal<void, ToggleButton *> onSelect;
		sigc::signal<void, ToggleButton *> onDeselect;
	private:
		int m_pressed;
	};
}

#endif /* _GUITOGGLEBUTTON_H */
