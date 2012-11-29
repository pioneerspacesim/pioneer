// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GAMEUI_SHIPSPINNER_H
#define GAMEUI_SHIPSPINNER_H

#include "ui/Context.h"
#include "ShipFlavour.h"
#include "graphics/Light.h"
#include "LmrModel.h"
#include "EquipSet.h"

namespace GameUI {

class ShipSpinner : public UI::Widget {
public:
	ShipSpinner(UI::Context *context, const ShipFlavour &flavour);

	virtual UI::Point PreferredSize() { return UI::Point(INT_MAX); }
	virtual void Layout();
	virtual void Update();
	virtual void Draw();

protected:
	virtual void HandleMouseDown(const UI::MouseButtonEvent &event);
	virtual void HandleMouseMove(const UI::MouseMotionEvent &event);

private:
	ShipFlavour m_flavour;

	float m_rotX, m_rotY;

	LmrModel *m_model;
	LmrObjParams m_params;
	// XXX m_equipment is currently not hooked up to anything,
	// it's just used to pass equipment parameters to the displayed model
	EquipSet m_equipment;
	//illumination in ship view
	Graphics::Light m_light;

	bool m_rightMouseButton;
};

}

#endif
