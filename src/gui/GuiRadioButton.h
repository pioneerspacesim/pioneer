// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIRADIOBUTTON_H
#define _GUIRADIOBUTTON_H

#include "GuiButton.h"
#include "GuiISelectable.h"
#include "GuiWidget.h"
#include <string>

namespace Gui {
	class RadioGroup;

	class RadioButton : public Button, public ISelectable {
	public:
		RadioButton(RadioGroup *);
		virtual ~RadioButton();
		virtual void Draw();
		virtual void GetSizeRequested(float size[2]);
		virtual bool OnMouseDown(MouseButtonEvent *e);
		virtual void OnActivate();
		virtual void SetSelected(bool state) { m_pressed = state; }
		bool GetSelected() { return m_pressed; }

	protected:
		bool m_pressed;
	};

} // namespace Gui

#endif /* _GUIRADIOBUTTON_H */
