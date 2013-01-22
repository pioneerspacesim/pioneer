// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SPACE_H
#define _SPACE_H

#include <list>
#include "Object.h"
#include "vector3.h"
#include "Serializer.h"
#include "RefCounted.h"
#include "galaxy/StarSystem.h"
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
	Frame *GetFrameByIndex(Uint32 idx) const;
	Body  *GetBodyByIndex(Uint32 idx) const;
	SystemBody *GetSystemBodyByIndex(Uint32 idx) const;
	Uint32 GetIndexForFrame(const Frame *frame) const;
	Uint32 GetIndexForBody(const Body *body) const;
	Uint32 GetIndexForSystemBody(const SystemBody *sbody) const;

	RefCountedPtr<StarSystem> GetStarSystem() const { return m_starSystem; }

	Frame *GetRootFrame() const { return m_rootFrame.Get(); }

	void AddBody(Body *);
	void RemoveBody(Body *);
	void KillBody(Body *);

	void TimeStep(float step);

	vector3d GetHyperspaceExitPoint(const SystemPath &source) const;

	Body *FindNearestTo(const Body *b, Object::Type t) const;
	Body *FindBodyForPath(const SystemPath *path) const;

	typedef std::list<Body*>::const_iterator BodyIterator;
	const BodyIterator BodiesBegin() const { return m_bodies.begin(); }
	const BodyIterator BodiesEnd() const { return m_bodies.end(); }

	Background::Container& GetBackground() { return m_background; }

	// body finder delegates
	typedef std::vector<Body*> BodyNearList;
	typedef BodyNearList::iterator BodyNearIterator;
	void GetBodiesMaybeNear(const Body *b, double dist, BodyNearList &bodies) const {
		m_bodyNearFinder.GetBodiesMaybeNear(b, dist, bodies);
	}
	void GetBodiesMaybeNear(const vector3d &pos, double dist, BodyNearList &bodies) const {
		m_bodyNearFinder.GetBodiesMaybeNear(pos, dist, bodies);
	}


private:
	void GenBody(SystemBody *b, Frame *f);
	// make sure SystemBody* is in Pi::currentSystem
	Frame *GetFrameWithSystemBody(const SystemBody *b) const;

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
	void RebuildSystemBodyIndex();

	void AddFrameToIndex(Frame *frame);
	void AddSystemBodyToIndex(SystemBody *sbody);

	bool m_frameIndexValid, m_bodyIndexValid, m_sbodyIndexValid;
	std::vector<Frame*> m_frameIndex;
	std::vector<Body*>  m_bodyIndex;
	std::vector<SystemBody*> m_sbodyIndex;

	//background (elements that are infinitely far away,
	//e.g. starfield and milky way)
	Background::Container m_background;

	class BodyNearFinder {
	public:
		BodyNearFinder(const Space *space) : m_space(space) {}
		void Prepare();

		void GetBodiesMaybeNear(const Body *b, double dist, BodyNearList &bodies) const;
		void GetBodiesMaybeNear(const vector3d &pos, double dist, BodyNearList &bodies) const;

	private:
		struct BodyDist {
			BodyDist(Body *_body, double _dist) : body(_body), dist(_dist) {}
			Body   *body;
			double dist;

			bool operator<(const BodyDist &a) const { return dist < a.dist; }

			friend bool operator<(const BodyDist &a, double d) { return a.dist < d; }
			friend bool operator<(double d, const BodyDist &a) { return d < a.dist; }
		};

		const Space *m_space;
		std::vector<BodyDist> m_bodyDist;
	};

	BodyNearFinder m_bodyNearFinder;

#ifndef NDEBUG
	//to check RemoveBody and KillBody are not called from within
	//the NotifyRemoved callback (#735)
	bool m_processingFinalizationQueue;
#endif

	void DebugDumpFrames();
};

#endif /* _SPACE_H */
