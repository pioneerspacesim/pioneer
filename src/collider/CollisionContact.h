// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLISION_CONTACT_H
#define _COLLISION_CONTACT_H

#include "vector3.h"

struct CollisionContact {
	/* position and normal are in world (or rather, CollisionSpace) coordinates */
	vector3d pos;
	vector3d normal;
	double depth;
	double dist; // distance travelled to hit point
	double timestep;
	int triIdx;
	void *userData1, *userData2;
	int geomFlag;
	//	bool vsStatic;		// true => object 2 was in static, else dynamic
	CollisionContact() : // default ctor
		depth(0),
		dist(0),
		timestep(0),
		triIdx(-1),
		userData1(nullptr),
		userData2(nullptr),
		geomFlag(0)
		{}
	CollisionContact(vector3d p, vector3d n, double de, double t, void *u1, void *u2) : // ctor for collision with terrain
		pos(p),
		normal(n),
		depth(de),
		timestep(t),
		userData1(u1),
		userData2(u2),
		geomFlag(0)
		{}
};

#endif /* _COLLISION_CONTACT_H */
