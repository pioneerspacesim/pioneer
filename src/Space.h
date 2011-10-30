#ifndef _SPACE_H
#define _SPACE_H

#include <list>
#include "Object.h"
#include "vector3.h"
#include "Serializer.h"

class Body;
class Frame;
class SBody;
class SystemPath;
class Ship;

// The place all the 'Body's exist in
namespace Space {
	extern void Init();
	extern void Uninit();
	extern void Clear();
	extern void BuildSystem();
	extern void Serialize(Serializer::Writer &wr);
	extern void Unserialize(Serializer::Reader &rd);
	extern void GenBody(SBody *b, Frame *f);
	extern void TimeStep(float step);
	extern void AddBody(Body *);
	extern void RemoveBody(Body *);
	extern void KillBody(Body *);
	extern void RadiusDamage(Body *attacker, Frame *f, const vector3d &pos, double radius, double kgDamage);
	extern void DoECM(const Frame *f, const vector3d &pos, int power_val);
	extern float GetHyperspaceAnim();
	extern const SystemPath *GetHyperspaceDest();
	extern double GetHyperspaceDuration();
	extern void StartHyperspaceTo(Ship *s, const SystemPath *);
	extern void DoHyperspaceTo(const SystemPath *);
	extern vector3d GetRandomPosition(float min_dist, float max_dist);
	extern vector3d GetPositionAfterHyperspace(const SystemPath *source, const SystemPath *dest);
	extern void SetupSystemForGameStart(const SystemPath *, int, int);
	// make sure SBody* is in Pi::currentSystem
	extern Frame *GetFrameWithSBody(const SBody *b);
	extern Body *FindNearestTo(const Body *b, Object::Type t);
	extern Body *FindBodyForPath(const SystemPath *path);

	extern std::list<Body*> bodies;
	typedef std::list<Body*>::iterator bodiesIter_t;
	extern Frame *rootFrame;
}


#endif /* _SPACE_H */
