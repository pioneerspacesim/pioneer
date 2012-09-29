// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _INTRO_H
#define _INTRO_H

#include "libs.h"
#include "Background.h"
#include "EquipSet.h"
#include "LmrModel.h"
#include "graphics/Light.h"

namespace Graphics {
	class Renderer;
}

class Intro {
public:
	Intro(Graphics::Renderer *r, int width, int height);
	void Draw(float time);

private:
	Color m_ambientColor;
	EquipSet m_equipment;
	float m_aspectRatio;
	Graphics::Renderer *m_renderer;
	LmrModel *m_model;
	LmrObjParams m_modelParams;
	ScopedPtr<Background::Container> m_background;
	std::vector<Graphics::Light> m_lights;
};

#endif
