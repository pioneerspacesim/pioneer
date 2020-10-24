// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "../View.h"
#include "PiGui.h"

// replacement for UIView. Currently a shim, this class should be able to
// dispatch to lua-registered handler functions and call them during the Draw3D
// method.
class PiGuiView : public View {
public:
	PiGuiView(std::string name) :
		m_handlerName(name) {}

	virtual void Update() override {}
	virtual void Draw3D() override {}
	virtual void DrawPiGui() override;

	const std::string &GetViewName() { return m_handlerName; }

private:
	virtual void OnSwitchTo() override{};

	std::string m_handlerName;
};
