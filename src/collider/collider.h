#ifndef _COLLISION_H
#define _COLLISION_H

#include "../vector3.h"
#include "GeomTree.h"
#include "CollisionSpace.h"
#include "Geom.h"

class Geom;

struct CollisionContact {
	vector3d pos;
	vector3d normal;
	double depth;
	int triIdx;
	Geom *g1, *g2;
};

#endif /* _COLLISION_H */
