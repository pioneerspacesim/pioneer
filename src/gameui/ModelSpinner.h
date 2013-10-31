// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef GAMEUI_MODELSPINNER_H
#define GAMEUI_MODELSPINNER_H

#include "ui/Context.h"
#include "graphics/Light.h"
#include "scenegraph/SceneGraph.h"
#include "scenegraph/ModelSkin.h"
#include "SmartPtr.h"

namespace GameUI {

class ModelSpinner : public UI::Widget {
public:
	ModelSpinner(UI::Context *context, SceneGraph::Model *model, const SceneGraph::ModelSkin &skin);

	virtual UI::Point PreferredSize() { return UI::Point(INT_MAX); }
	virtual void Layout();
	virtual void Update();
	virtual void Draw();

protected:
	virtual void HandleMouseDown(const UI::MouseButtonEvent &event);
	virtual void HandleMouseMove(const UI::MouseMotionEvent &event);

private:
	std::unique_ptr<SceneGraph::Model> m_model;
	SceneGraph::ModelSkin m_skin;

	float m_rotX, m_rotY;

	Graphics::Light m_light;

	bool m_rightMouseButton;
};

}

#endif
