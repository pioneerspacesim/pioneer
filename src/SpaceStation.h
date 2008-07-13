#ifndef _SPACESTATION_H
#define _SPACESTATION_H

#include "libs.h"
#include "ModelBody.h"

class SpaceStation: public ModelBody {
public:
	SpaceStation();
	virtual ~SpaceStation();
	virtual bool OnCollision(Body *b);
	virtual Object::Type GetType() { return Object::SPACESTATION; }
	virtual void Render(const Frame *camFrame);
protected:
};

#endif /* _SPACESTATION_H */
