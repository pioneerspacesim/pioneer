#ifndef _PLANET_H
#define _PLANET_H

#include "TerrainBody.h"
// only for SBody::BodySuperType enum...
#include "StarSystem.h"

class Frame;
class SBody;
class GeoSphere;

class Planet: public TerrainBody {
public:
	OBJDEF(Planet, TerrainBody, PLANET);
	Planet(SBody*);
	Planet();
	virtual ~Planet() {}

	void GetAtmosphericState(double dist, double *outPressure, double *outDensity);

#if OBJECTVIEWER
	friend class ObjectViewerView;
#endif

private:
	void DrawGasGiantRings();
	void DrawAtmosphere(vector3d &pos);

	GLuint m_ringsDList;
};

#endif /* _PLANET_H */
