#ifndef _SPACESTATION_H
#define _SPACESTATION_H

#include "libs.h"
#include "ModelBody.h"

class CollMeshSet;

class SpaceStation: public ModelBody {
public:
	SpaceStation();
	virtual ~SpaceStation();
	virtual bool OnCollision(Body *b, Uint32 flags);
	virtual Object::Type GetType() { return Object::SPACESTATION; }
	virtual void Render(const Frame *camFrame);
	void GetDockingSurface(CollMeshSet *mset, int midx);
	struct dockingport_t {
		vector3d center;
		vector3d normal;
		vector3d horiz;
	} port;
private:
	bool allowDocking;
};

#endif /* _SPACESTATION_H */
