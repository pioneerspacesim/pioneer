// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SPACE_H
#define _SPACE_H

#include "Background.h"
#include "FrameId.h"
#include "IterationProxy.h"
#include "RefCounted.h"
#include "galaxy/StarSystem.h"
#include "vector3.h"

class Body;
class Frame;
class Game;
enum class ObjectType;

class Space {
public:
	// empty space (eg for hyperspace)
	Space(Game *game, RefCountedPtr<Galaxy> galaxy, Space *oldSpace = nullptr);

	// initialise with system bodies
	Space(Game *game, RefCountedPtr<Galaxy> galaxy, const SystemPath &path, Space *oldSpace = nullptr);

	// initialise from save file
	Space(Game *game, RefCountedPtr<Galaxy> galaxy, const Json &jsonObj, double at_time);

	~Space();

	void ToJson(Json &jsonObj);

	// body/sbody indexing for save/load. valid after
	// construction/ToJson(), invalidated by TimeStep(). they will assert
	// if called while invalid
	Body *GetBodyByIndex(Uint32 idx) const;
	SystemBody *GetSystemBodyByIndex(Uint32 idx) const;
	Uint32 GetIndexForBody(const Body *body) const;
	Uint32 GetIndexForSystemBody(const SystemBody *sbody) const;

	RefCountedPtr<StarSystem> GetStarSystem() const { return m_starSystem; }

	FrameId GetRootFrame() const { return m_rootFrameId; }

	void AddBody(Body *);
	void RemoveBody(Body *);
	void KillBody(Body *);

	void TimeStep(float step);

	void GetHyperspaceExitParams(const SystemPath &source, const SystemPath &dest,
		vector3d &pos, vector3d &vel) const;
	vector3d GetHyperspaceExitPoint(const SystemPath &source, const SystemPath &dest) const
	{
		vector3d pos, vel;
		GetHyperspaceExitParams(source, dest, pos, vel);
		return pos;
	}
	vector3d GetHyperspaceExitPoint(const SystemPath &source) const
	{
		return GetHyperspaceExitPoint(source, m_starSystem->GetPath());
	}

	Body *FindNearestTo(const Body *b, ObjectType t) const;
	Body *FindBodyForPath(const SystemPath *path) const;

	Uint32 GetNumBodies() const { return static_cast<Uint32>(m_bodies.size()); }
	IterationProxy<std::vector<Body *>> GetBodies() { return MakeIterationProxy(m_bodies); }
	const IterationProxy<const std::vector<Body *>> GetBodies() const { return MakeIterationProxy(m_bodies); }

	Background::Container *GetBackground() { return m_background.get(); }
	void RefreshBackground();

	// body finder delegates
	typedef const std::vector<Body *> BodyNearList;
	BodyNearList GetBodiesMaybeNear(const Body *b, double dist)
	{
		return m_bodyNearFinder.GetBodiesMaybeNear(b, dist);
	}
	BodyNearList GetBodiesMaybeNear(const vector3d &pos, double dist)
	{
		return m_bodyNearFinder.GetBodiesMaybeNear(pos, dist);
	}

	void DebugDumpFrames(bool details);

	struct BodyDist {
		BodyDist(Body *_body, double _dist) :
			body(_body),
			dist(_dist) {}
		Body *body;
		double dist;

		bool operator<(const BodyDist &a) const { return dist < a.dist; }

		friend bool operator<(const BodyDist &a, double d) { return a.dist < d; }
		friend bool operator<(double d, const BodyDist &a) { return d < a.dist; }
	};

	//Find bodies within angle to given direction. dir and offset relative to b like in ship coordinates
	//returns unsorted vector of bodies with their distance from b+offset
	//It calculates distance from b for all the bodies so it is quite inefficiet,
	//it is not designed to be called in each game loop!
	std::vector<BodyDist> BodiesInAngle(const Body *b, const vector3d &offset, const vector3d &dir, double cosOfMaxAngle) const;

private:
	void GenSectorCache(RefCountedPtr<Galaxy> galaxy, const SystemPath *here);
	void UpdateStarSystemCache(const SystemPath *here);
	void GenBody(const double at_time, SystemBody *b, FrameId fId, std::vector<vector3d> &posAccum);
	// make sure SystemBody* is in Pi::currentSystem
	FrameId GetFrameWithSystemBody(const SystemBody *b) const;

	void UpdateBodies();

	void CollideFrame(FrameId fId);

	FrameId m_rootFrameId;

	RefCountedPtr<SectorCache::Slave> m_sectorCache;
	RefCountedPtr<StarSystemCache::Slave> m_starSystemCache;

	RefCountedPtr<StarSystem> m_starSystem;

	Game *m_game;

	// all the bodies we know about
	std::vector<Body *> m_bodies;

	// bodies that were removed/killed this timestep and need pruning at the end
	enum class BodyAssignation {
		KILL = 0,
		REMOVE = 1
	};

	std::vector<std::pair<Body *, BodyAssignation>> m_assignedBodies;

	void RebuildBodyIndex();
	void RebuildSystemBodyIndex();

	void AddSystemBodyToIndex(SystemBody *sbody);

	bool m_bodyIndexValid, m_sbodyIndexValid;
	std::vector<Body *> m_bodyIndex;
	std::vector<SystemBody *> m_sbodyIndex;

	//background (elements that are infinitely far away,
	//e.g. starfield and milky way)
	std::unique_ptr<Background::Container> m_background;

	class BodyNearFinder {
	public:
		BodyNearFinder(const Space *space) :
			m_space(space) {}
		void Prepare();

		BodyNearList GetBodiesMaybeNear(const Body *b, double dist);
		BodyNearList GetBodiesMaybeNear(const vector3d &pos, double dist);

	private:
		const Space *m_space;
		std::vector<BodyDist> m_bodyDist;
		std::vector<Body *> m_nearBodies;
	};

	BodyNearFinder m_bodyNearFinder;

#ifndef NDEBUG
	//to check RemoveBody and KillBody are not called from within
	//the NotifyRemoved callback (#735)
	bool m_processingFinalizationQueue;
#endif
};

#endif /* _SPACE_H */
