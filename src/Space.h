#ifndef _SPACE_H
#define _SPACE_H

#include <list>
#include "vector3.h"

class Body;
class Frame;
class SBody;
class Ship;

// The place all the 'Body's exist in
namespace Space {
	extern void Init();
	extern void Clear();
	extern void BuildSystem();
	extern void Serialize();
	extern void Unserialize();
	extern void GenBody(SBody *b, Frame *f);
	extern void TimeStep(float step);
	extern void AddBody(Body *);
	extern void RemoveBody(Body *);
	extern void KillBody(Body *);
	extern void Render(const Frame *cam_frame);
	extern void AddLaserBeam(Frame *f, const vector3d &pos, const vector3d &dir,
		double length, Ship *firer, float damage);
	// make sure SBody* is in Pi::currentSystem
	extern Frame *GetFrameWithSBody(const SBody *b);

	extern std::list<Body*> bodies;
	typedef std::list<Body*>::iterator bodiesIter_t;
	extern Frame *rootFrame;
}


#endif /* _SPACE_H */
