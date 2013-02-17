// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIPSPINNERWIDGET_H
#define _SHIPSPINNERWIDGET_H

#include "gui/Gui.h"
#include "graphics/Light.h"
#include "scenegraph/SceneGraph.h"
#include "SmartPtr.h"

class ShipSpinnerWidget : public Gui::Widget {
public:
	ShipSpinnerWidget(SceneGraph::Model *model, float width, float height);

	virtual void Draw();
	virtual void GetSizeRequested(float size[2]) { size[0] = m_width; size[1] = m_height; }

private:
	float m_width;
	float m_height;

	ScopedPtr<SceneGraph::Model> m_model;

	Graphics::Light m_light;
};

#endif
