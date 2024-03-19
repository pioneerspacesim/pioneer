// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLISION_CONTACT_H
#define _COLLISION_CONTACT_H

#include "vector3.h"

struct CollisionContact {
	// position and normal are in world (or rather, CollisionSpace) coordinates
	vector3d pos;
	vector3d normal;
	double depth;
	double distance; // distance travelled to hit point
	double timestep;
	int triIdx;
	int geomFlag;
	void *userData1, *userData2;

	// default ctor
	CollisionContact() :
		depth(0),
		distance(0),
		timestep(0),
		triIdx(-1),
		geomFlag(0),
		userData1(nullptr),
		userData2(nullptr)
	{}

	// ctor for collision with terrain
	CollisionContact(const vector3d &position, const vector3d &normal, double deep, double dt, void *u1, void *u2) :
		pos(position),
		normal(normal),
		depth(deep),
		distance(0),
		timestep(dt),
		triIdx(-1),
		userData1(u1),
		userData2(u2),
		geomFlag(0)
	{}
};

#endif /* _COLLISION_CONTACT_H */
