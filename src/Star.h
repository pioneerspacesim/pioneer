// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _STAR_H
#define _STAR_H

#include "TerrainBody.h"
#include "graphics/RenderState.h"

namespace Graphics {
	class Renderer;
}

class Star : public TerrainBody {
public:
	OBJDEF(Star, TerrainBody, STAR);
	Star() = delete;
	Star(SystemBody *sbody);
	Star(const Json &jsonObj, Space *space);
	virtual ~Star(){};

	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;

protected:
	void InitStar();

	Graphics::RenderState *m_haloState;
};

#endif /* _STAR_H */
