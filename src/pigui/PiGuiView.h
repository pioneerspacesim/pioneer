// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "../View.h"

// Replacement for UIView. This class dispatches to lua-registered pigui draw
// functions when DrawPiGui() is called.
// TODO: support rendering to a debug window during the Update() and Draw3D() methods.
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
