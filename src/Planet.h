// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _PLANET_H
#define _PLANET_H

#include "TerrainBody.h"
#include "graphics/VertexArray.h"
#include "SmartPtr.h"

namespace Graphics {
	class Renderer;
	class RenderState;
	class Texture;
	class Material;
}

class Planet: public TerrainBody {
public:
	OBJDEF(Planet, TerrainBody, PLANET);
	Planet(SystemBody*);
	Planet();

	virtual void SubRender(Graphics::Renderer *r, const matrix4x4d &viewTran, const vector3d &camPos);

	void GetAtmosphericState(double dist, double *outPressure, double *outDensity) const;
	double GetAtmosphereRadius() const { return m_atmosphereRadius; }

#if WITH_OBJECTVIEWER
	friend class ObjectViewerView;
#endif

protected:
	virtual void Load(Serializer::Reader &rd, Space *space);

private:
	void InitParams(const SystemBody*);
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
