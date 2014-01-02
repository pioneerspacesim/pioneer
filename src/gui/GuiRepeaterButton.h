// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GUIREPEATERBUTTON_H
#define _GUIREPEATERBUTTON_H

#include "GuiButton.h"

namespace Gui {
	/* Emits click events like the normal button, except that when
	 * depressed for msDelay milliseconds it will begin emitting events
	 * every msRepeat milliseconds */
	class RepeaterButton: public SolidButton {
	public:
		RepeaterButton(int msDelay, int msRepeat);
		int GetRepeat();
		void SetRepeat(int msRepeat);
		virtual ~RepeaterButton();
	private:
		void OnPress();
		void OnRelease();
		void OnRepeat();
		sigc::connection m_repeatCon;
		int m_delay, m_repeat;
	};
}

#endif /* _GUIREPEATERBUTTON_H */
