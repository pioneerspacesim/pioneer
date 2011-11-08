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
class HyperspaceCloud;

class Space {
public:
	Space(const SystemPath &path);
	Space(Serializer::Reader &rd);
	virtual ~Space();

	//void Save(Serializer::Writer &wr);

	StarSystem *GetStarSystem() const { return m_starSystem; }

	Frame *GetRootFrame() const { return rootFrame; }

	void AddBody(Body *);
	void RemoveBody(Body *);
	void KillBody(Body *);

	void TimeStep(float step);

	// XXX these do not belong here
	static vector3d GetRandomPosition(float min_dist, float max_dist);
	static vector3d GetPositionAfterHyperspace(const SystemPath *source, const SystemPath *dest);

	// XXX these may belong elsewhere
	void RadiusDamage(Body *attacker, Frame *f, const vector3d &pos, double radius, double kgDamage);
	void DoECM(const Frame *f, const vector3d &pos, int power_val);

	void StartHyperspaceTo(Ship *s, const SystemPath *);
	void DoHyperspaceTo(const SystemPath *);

	Body *FindNearestTo(const Body *b, Object::Type t);
	Body *FindBodyForPath(const SystemPath *path);

	float GetHyperspaceAnim() const { return hyperspaceAnim; }
	const SystemPath *GetHyperspaceDest() const { return hyperspacingTo; }
	double GetHyperspaceDuration() const { return hyperspaceDuration; }

	// XXX make private
	std::list<Body*> bodies;
	typedef std::list<Body*>::iterator bodiesIter_t;
	Frame *rootFrame;

private:
	void GenBody(SBody *b, Frame *f);
	// make sure SBody* is in Pi::currentSystem
	Frame *GetFrameWithSBody(const SBody *b);

	void PruneCorpses();

	void CollideFrame(Frame *f);

	StarSystem *m_starSystem;

	std::list<Body*> corpses;
	SystemPath *hyperspacingTo;
	float hyperspaceAnim;
	double hyperspaceDuration;
	double hyperspaceEndTime;
	std::list<HyperspaceCloud*> storedArrivalClouds;
};

#endif /* _SPACE_H */
