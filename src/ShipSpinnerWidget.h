// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPSPINNERWIDGET_H
#define _SHIPSPINNERWIDGET_H

#include "EquipSet.h"
#include "ShipFlavour.h"
#include "ShipType.h"
#include "gui/Gui.h"
#include "graphics/Light.h"

namespace SceneGraph { class Model; }

class ShipSpinnerWidget : public Gui::Widget {
public:
	ShipSpinnerWidget(const ShipFlavour &flavour, float width, float height);

	virtual void Draw();
	virtual void GetSizeRequested(float size[2]) { size[0] = m_width; size[1] = m_height; }

private:
	float m_width;
	float m_height;

	SceneGraph::Model *m_model;
	// XXX m_equipment is currently not hooked up to anything,
	// it's just used to pass equipment parameters to the displayed model
	EquipSet m_equipment;
	//illumination in ship view
	Graphics::Light m_light;
};

#endif
