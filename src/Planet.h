#ifndef _PLANET_H
#define _PLANET_H

#include "TerrainBody.h"

class Planet: public TerrainBody {
public:
	OBJDEF(Planet, TerrainBody, PLANET);
	Planet(SBody*);
	Planet();
	virtual ~Planet() {}

	virtual void SubRender(const vector3d &camPos);

	void GetAtmosphericState(double dist, double *outPressure, double *outDensity);

#if OBJECTVIEWER
	friend class ObjectViewerView;
#endif

private:
	void DrawGasGiantRings();
	void DrawAtmosphere(const vector3d &camPos);

	GLuint m_ringsDList;
};

#endif /* _PLANET_H */
