#ifndef _AABB_H
#define _AABB_H

#include "vector3.h"

struct Aabb {
	vector3d max, min;
	void Update(vector3d p) {
		if (max.x < p.x) max.x = p.x;
		if (max.y < p.y) max.y = p.y;
		if (max.z < p.z) max.z = p.z;
		if (min.x > p.x) min.x = p.x;
		if (min.y > p.y) min.y = p.y;
		if (min.z > p.z) min.z = p.z;
	}
};

#endif /* _AABB_H */
