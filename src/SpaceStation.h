#ifndef _SPACESTATION_H
#define _SPACESTATION_H

#include "libs.h"
#include "ModelBody.h"

class CollMeshSet;
class Ship;

class SpaceStation: public ModelBody {
public:
	enum TYPE { JJHOOP, GROUND_FLAVOURED, TYPE_MAX };
	SpaceStation(TYPE);
	virtual ~SpaceStation();
	virtual bool OnCollision(Body *b, Uint32 flags);
	virtual Object::Type GetType() { return Object::SPACESTATION; }
	virtual void Render(const Frame *camFrame);
	void OrientLaunchingShip(Ship *ship) const;
	void OrientDockedShip(Ship *ship) const;
	void GetDockingSurface(CollMeshSet *mset, int midx);
	bool GetDockingClearance(Ship *s);
	bool IsGroundStation() const;
	struct dockingport_t {
		vector3d center;
		vector3d normal;
		vector3d horiz;
	} port;
private:
	TYPE m_type;
};

#endif /* _SPACESTATION_H */
