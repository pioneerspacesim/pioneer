#ifndef _SPACE_H
#define _SPACE_H

#include <list>
#include "Object.h"
#include "vector3.h"
#include "Serializer.h"
#include "RefCounted.h"
#include "StarSystem.h"

class Body;
class Frame;
class Ship;
class HyperspaceCloud;

class Space {
public:
	// empty space (eg for hyperspace)
	Space();

	// initalise with system bodies
	Space(const SystemPath &path);

	// initialise from save file
	Space(Serializer::Reader &rd);

	virtual ~Space();

	void Serialize(Serializer::Writer &wr);

	RefCountedPtr<StarSystem> GetStarSystem() const { return m_starSystem; }

	Frame *GetRootFrame() const { return m_rootFrame.Get(); }

	void AddBody(Body *);
	void RemoveBody(Body *);
	void KillBody(Body *);

	void TimeStep(float step);

	vector3d GetHyperspaceExitPoint(const SystemPath &source);

	Body *FindNearestTo(const Body *b, Object::Type t);
	Body *FindBodyForPath(const SystemPath *path);

	typedef std::list<Body*>::const_iterator BodyIterator;
	const BodyIterator IteratorBegin() const { return m_bodies.begin(); }
	const BodyIterator IteratorEnd() const { return m_bodies.end(); }

private:
	void GenBody(SBody *b, Frame *f);
	// make sure SBody* is in Pi::currentSystem
	Frame *GetFrameWithSBody(const SBody *b);

	void UpdateBodies();

	void CollideFrame(Frame *f);

	ScopedPtr<Frame> m_rootFrame;

	RefCountedPtr<StarSystem> m_starSystem;

	// all the bodies we know about
	std::list<Body*> m_bodies;

	// bodies that were removed/killed this timestep and need pruning at the end
	std::list<Body*> m_removeBodies;
	std::list<Body*> m_killBodies;
};

#endif /* _SPACE_H */
