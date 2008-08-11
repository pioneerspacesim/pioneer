#ifndef _SPACESTATION_H
#define _SPACESTATION_H

#include "libs.h"
#include "ModelBody.h"

class CollMeshSet;
class Ship;

class SpaceStation: public ModelBody {
public:
	SpaceStation();
	virtual ~SpaceStation();
	virtual bool OnCollision(Body *b, Uint32 flags);
	virtual Object::Type GetType() { return Object::SPACESTATION; }
	virtual void Render(const Frame *camFrame);
	void GetDockingSurface(CollMeshSet *mset, int midx);
	bool GetDockingClearance(Ship *s);
	struct dockingport_t {
		vector3d center;
		vector3d normal;
		vector3d horiz;
	} port;
};

#endif /* _SPACESTATION_H */
