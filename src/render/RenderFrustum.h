#ifndef _FRUSTUM_H
#define _FRUSTUM_H

#include "libs.h"
#include "vector3.h"

namespace Render {

class Frustum {
public:
	Frustum(float width, float height, float fovAng);

	void Enable();
	void Disable();

	void SetFov(float ang);

	bool TestPoint(const vector3d &p, double radius) const;
	bool TestPointInfinite(const vector3d &p, double radius) const;

	bool ProjectPoint(vector3d &in, vector3d &out) const;

	inline void Update() { Update(false); }

private:
	struct Plane {
		double a, b, c, d;
		double DistanceToPoint(const vector3d &p) const {
			return a*p.x + b*p.y + c*p.z + d;
		}
	};

	void Update(bool force);

	float m_width;
	float m_height;

	bool m_shadersEnabled;
	double m_fov;

	GLfloat m_frustumLeft;
	GLfloat m_frustumTop;

	GLdouble m_modelMatrix[16];
	GLdouble m_projMatrix[16];
	GLint m_viewport[4];

	Plane m_planes[6];
};

}

#endif
