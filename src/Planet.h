// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _PLANET_H
#define _PLANET_H

#include "SmartPtr.h"
#include "TerrainBody.h"
#include "graphics/VertexArray.h"

namespace Graphics {
	class Renderer;
	class RenderState;
	class Texture;
	class Material;
} // namespace Graphics

class Planet : public TerrainBody {
public:
	OBJDEF(Planet, TerrainBody, PLANET);
	Planet() = delete;
	Planet(SystemBody *);
	Planet(const Json &jsonObj, Space *space);

	virtual void SubRender(Graphics::Renderer *r, const matrix4x4d &viewTran, const vector3d &camPos) override;

	void GetAtmosphericState(double dist, double *outPressure, double *outDensity) const;
	double GetAtmosphereRadius() const { return m_atmosphereRadius; }

	friend class ObjectViewerView;

protected:
private:
	void InitParams();
	void GenerateRings(Graphics::Renderer *renderer);
	void DrawGasGiantRings(Graphics::Renderer *r, const matrix4x4d &modelView);

	double m_atmosphereRadius;
	double m_surfaceGravity_g;
	RefCountedPtr<Graphics::Texture> m_ringTexture;
	Graphics::VertexArray m_ringVertices;
	std::unique_ptr<Graphics::Material> m_ringMaterial;
	Graphics::RenderState *m_ringState;

	// Legacy renderer visuals
	std::unique_ptr<Graphics::VertexArray> m_atmosphereVertices;
	std::unique_ptr<Graphics::Material> m_atmosphereMaterial;
};

#endif /* _PLANET_H */
