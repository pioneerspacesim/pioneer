#ifndef _PLANET_H
#define _PLANET_H

#include "TerrainBody.h"

class Planet: public TerrainBody {
public:
	OBJDEF(Planet, TerrainBody, PLANET);
	Planet(SBody*);
	Planet();
	virtual ~Planet() {}

	virtual void SubRender(Renderer *r, const vector3d &camPos);

	void GetAtmosphericState(double dist, double *outPressure, double *outDensity);

#if WITH_OBJECTVIEWER
	friend class ObjectViewerView;
#endif

private:
	void DrawGasGiantRings(Renderer *r);
	void DrawAtmosphere(Renderer *r, const vector3d &camPos);

	GLuint m_ringsDList;
};

#endif /* _PLANET_H */
