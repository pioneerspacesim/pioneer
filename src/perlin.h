#ifndef _PERLIN_H
#define _PERLIN_H

#include "vector3.h"

double noise(const double x, const double y, const double z );
static inline double noise(const vector3d &p) {
	return noise(p.x, p.y, p.z);
}
void perlin_init();

#endif /* _PERLIN_H */
