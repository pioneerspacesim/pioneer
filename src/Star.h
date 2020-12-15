// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _STAR_H
#define _STAR_H

#include "TerrainBody.h"

class Camera;
class Space;
class SystemBody;

namespace Graphics {
	class Renderer;
	class RenderState;
	class VertexBuffer;
} // namespace Graphics

class Star : public TerrainBody {
public:
	OBJDEF(Star, TerrainBody, STAR);
	Star() = delete;
	Star(SystemBody *sbody);
	Star(const Json &jsonObj, Space *space);
	virtual ~Star();

	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;

protected:
	void InitStar();
	void BuildHaloBuffer(Graphics::Renderer *renderer, double rad);

	Graphics::RenderState *m_haloState;
	std::unique_ptr<Graphics::VertexBuffer> m_haloBuffer;
};

#endif /* _STAR_H */
