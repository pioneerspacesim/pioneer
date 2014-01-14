// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

// DEPRECATED due to new ui system

#ifndef _SHIPSPINNERWIDGET_H
#define _SHIPSPINNERWIDGET_H

#include "gui/Gui.h"
#include "graphics/Light.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/ModelSkin.h"
#include "SmartPtr.h"
#include "Shields.h"

class Shields;

class ShipSpinnerWidget : public Gui::Widget {
public:
	ShipSpinnerWidget(SceneGraph::Model *model, const SceneGraph::ModelSkin &skin, float width, float height);

	virtual void Draw();
	virtual void GetSizeRequested(float size[2]) { size[0] = m_width; size[1] = m_height; }

private:
	float m_width;
	float m_height;

	std::unique_ptr<SceneGraph::Model> m_model;
	SceneGraph::ModelSkin m_skin;

	Graphics::Light m_light;
	std::unique_ptr<Shields> m_shields;
};

#endif
