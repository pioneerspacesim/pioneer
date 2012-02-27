#ifndef _SPACE_H
#define _SPACE_H

#include <list>
#include "Object.h"
#include "vector3.h"
#include "Serializer.h"
#include "RefCounted.h"
#include "StarSystem.h"
#include "Background.h"

class Body;
class Frame;
class Ship;
class HyperspaceCloud;
class Game;

class Space {
public:
	// empty space (eg for hyperspace)
	Space(Game *game);

	// initalise with system bodies
	Space(Game *game, const SystemPath &path);

	// initialise from save file
	Space(Game *game, Serializer::Reader &rd);

	virtual ~Space();

	void Serialize(Serializer::Writer &wr);

	// frame/body/sbody indexing for save/load. valid after
	// construction/Serialize(), invalidated by TimeStep(). they will assert
	// if called while invalid
	Frame *GetFrameByIndex(Uint32 idx);
	Body  *GetBodyByIndex(Uint32 idx);
	SBody *GetSBodyByIndex(Uint32 idx);
	Uint32 GetIndexForFrame(const Frame *frame);
	Uint32 GetIndexForBody(const Body *body);
	Uint32 GetIndexForSBody(const SBody *sbody);

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
	const BodyIterator BodiesBegin() const { return m_bodies.begin(); }
	const BodyIterator BodiesEnd() const { return m_bodies.end(); }

	const Background::Container& GetBackground() const { return m_background; }

private:
	void GenBody(SBody *b, Frame *f);
	// make sure SBody* is in Pi::currentSystem
	Frame *GetFrameWithSBody(const SBody *b);

	void UpdateBodies();

	void CollideFrame(Frame *f);

	ScopedPtr<Frame> m_rootFrame;

	RefCountedPtr<StarSystem> m_starSystem;

	Game *m_game;

	// all the bodies we know about
	std::list<Body*> m_bodies;

	// bodies that were removed/killed this timestep and need pruning at the end
	std::list<Body*> m_removeBodies;
	std::list<Body*> m_killBodies;

	void RebuildFrameIndex();
	void RebuildBodyIndex();
	void RebuildSBodyIndex();

	void AddFrameToIndex(Frame *frame);
	void AddSBodyToIndex(SBody *sbody);

	bool m_frameIndexValid, m_bodyIndexValid, m_sbodyIndexValid;
	std::vector<Frame*> m_frameIndex;
	std::vector<Body*>  m_bodyIndex;
	std::vector<SBody*> m_sbodyIndex;

	//background (elements that are infinitely far away,
	//e.g. starfield and milky way)
	Background::Container m_background;

#ifndef NDEBUG
	//to check RemoveBody and KillBody are not called from within
	//the NotifyRemoved callback (#735)
	bool m_processingFinalizationQueue;
#endif

	void DebugDumpFrames();
};

#endif /* _SPACE_H */
