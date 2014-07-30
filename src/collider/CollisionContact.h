// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _COLLISION_CONTACT_H
#define _COLLISION_CONTACT_H

struct CollisionContact {
	/* position and normal are in world (or rather, CollisionSpace) coordinates */
	vector3d pos;
	vector3d normal;
	double depth;
	double dist; // distance travelled to hit point
	int triIdx;
	void *userData1, *userData2;
	int geomFlag;
//	bool vsStatic;		// true => object 2 was in static, else dynamic
	CollisionContact() {
		depth = 0; triIdx = -1; userData1 = userData2 = 0; geomFlag = 0; dist = 0;
	}
};

#endif /* _COLLISION_CONTACT_H */
