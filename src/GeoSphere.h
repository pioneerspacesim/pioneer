#ifndef _GEOSPHERE_H
#define _GEOSPHERE_H

#include "vector3.h"
#include "mtrand.h"

class GeoPatch;
class GeoSphere {
public:
	GeoSphere();
	~GeoSphere();
	void Render(vector3d campos);
	inline vector3d GenPoint(int x, int y, const GeoPatch*, double *height);
	void SetColor(const float col[4]);
	void AddCraters(MTRand &rand, int num, double minAng, double maxAng);
	double GetHeight(const vector3d &p);
private:
	GeoPatch *m_patches[6];
	struct crater_t {
		vector3d pos;
		double size;
	} *m_craters;
	int m_numCraters;
	float m_diffColor[4], m_ambColor[4];
};

#endif /* _GEOSPHERE_H */
