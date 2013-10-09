// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GAMEUI_BINDINGCAPTURE_H
#define GAMEUI_BINDINGCAPTURE_H

#include "libs.h"
#include "ui/Context.h"
#include "KeyBindings.h"

namespace GameUI {

class KeyBindingCapture : public UI::Single {
public:
	KeyBindingCapture(UI::Context *context);

	virtual bool IsSelectable() const { return true; }

	const KeyBindings::KeyBinding &GetBinding() const { return m_binding; }
	void Capture();

	sigc::signal<void,const KeyBindings::KeyBinding &> onCapture;

protected:
	virtual void HandleKeyDown(const UI::KeyboardEvent &event);

private:
	KeyBindings::KeyBinding m_binding;
	bool m_capturing;
};

}

#endif
