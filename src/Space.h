// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
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

#ifndef NDEBUG
	//to check RemoveBody and KillBody are not called from within
	//the NotifyRemoved callback (#735)
	bool m_processingFinalizationQueue;
#endif

	void DebugDumpFrames();
};

#endif /* _SPACE_H */
