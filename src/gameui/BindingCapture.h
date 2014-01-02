// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
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
	~KeyBindingCapture();

	virtual bool IsSelectable() const { return true; }

	const KeyBindings::KeyBinding &GetBinding() const { return m_binding; }

	sigc::signal<void,const KeyBindings::KeyBinding &> onCapture;

protected:
	virtual void HandleVisible();
	virtual void HandleInvisible();
	virtual void HandleKeyDown(const UI::KeyboardEvent &event);

private:
	void Connect();
	void Disconnect();
	bool OnJoystickHatMove(const UI::JoystickHatMotionEvent &event);
	bool OnJoystickButtonDown(const UI::JoystickButtonEvent &event);

	sigc::connection m_connJoystickHatMove;
	sigc::connection m_connJoystickButtonDown;
	KeyBindings::KeyBinding m_binding;
};

class AxisBindingCapture : public UI::Single {
public:
	AxisBindingCapture(UI::Context *context);
	~AxisBindingCapture();

	virtual bool IsSelectable() const { return true; }

	const KeyBindings::AxisBinding &GetBinding() const { return m_binding; }

	sigc::signal<void,const KeyBindings::AxisBinding &> onCapture;

protected:
	virtual void HandleVisible();
	virtual void HandleInvisible();

private:
	void Connect();
	void Disconnect();
	bool OnJoystickAxisMove(const UI::JoystickAxisMotionEvent &event);

	sigc::connection m_connJoystickAxisMove;
	KeyBindings::AxisBinding m_binding;
};

}

#endif
