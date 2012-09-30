// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _TOMBSTONE_H
#define _TOMBSTONE_H

#include "libs.h"
#include "Background.h"
#include "LmrModel.h"
#include "graphics/Light.h"

namespace Graphics {
	class Renderer;
}

class Tombstone {
public:
	Tombstone(Graphics::Renderer *r, int width, int height);
	void Draw(float time);

private:
	Color m_ambientColor;
	float m_aspectRatio;
	Graphics::Renderer *m_renderer;
	LmrModel *m_model;
	LmrObjParams m_modelParams;
	std::vector<Graphics::Light> m_lights;
};

#endif
