// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GAMEUI_MODELSPINNER_H
#define GAMEUI_MODELSPINNER_H

#include "ui/Context.h"
#include "graphics/Light.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/ModelSkin.h"
#include "SmartPtr.h"
#include "Shields.h"

namespace GameUI {

class ModelSpinner : public UI::Widget {
public:
	ModelSpinner(UI::Context *context, SceneGraph::Model *model, const SceneGraph::ModelSkin &skin, unsigned int pattern);

	virtual UI::Point PreferredSize() { return UI::Point(INT_MAX); }
	virtual void Layout();
	virtual void Update();
	virtual void Draw();

	SceneGraph::Model *GetModel() const { return m_model.get(); }

protected:
	virtual void HandleMouseDown(const UI::MouseButtonEvent &event);
	virtual void HandleMouseMove(const UI::MouseMotionEvent &event);

private:
	std::unique_ptr<SceneGraph::Model> m_model;
	SceneGraph::ModelSkin m_skin;
	std::unique_ptr<Shields> m_shields;

	float m_rotX, m_rotY;

	Graphics::Light m_light;

	bool m_rightMouseButton;
};

}

#endif
