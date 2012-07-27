#ifndef _PLANET_H
#define _PLANET_H

#include "TerrainBody.h"
#include "graphics/VertexArray.h"
#include "SmartPtr.h"

namespace Graphics {
	class Renderer;
	class Texture;
}

class Planet: public TerrainBody {
public:
	OBJDEF(Planet, TerrainBody, PLANET);
	Planet(SystemBody*);
	Planet();
	virtual ~Planet();

	virtual double GetClipRadius() const { return m_clipRadius; }
	virtual void SubRender(Graphics::Renderer *r, const vector3d &camPos);

	void GetAtmosphericState(double dist, double *outPressure, double *outDensity) const;

#if WITH_OBJECTVIEWER
	friend class ObjectViewerView;
#endif

protected:
	virtual void Load(Serializer::Reader &rd, Space *space);

private:
	void GenerateRings();
	void DrawGasGiantRings(Graphics::Renderer *r);
	void DrawAtmosphere(Graphics::Renderer *r, const vector3d &camPos);

	double m_clipRadius;
	RefCountedPtr<Graphics::Texture> m_ringTexture;
	Graphics::VertexArray m_ringVertices;
};

#endif /* _PLANET_H */
