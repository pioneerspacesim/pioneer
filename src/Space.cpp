// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Space.h"
#include "Body.h"
#include "Frame.h"
#include "Star.h"
#include "Planet.h"
#include <algorithm>
#include <functional>
#include "Pi.h"
#include "Player.h"
#include "galaxy/StarSystem.h"
#include "SpaceStation.h"
#include "Serializer.h"
#include "collider/collider.h"
#include "Missile.h"
#include "HyperspaceCloud.h"
#include "graphics/Graphics.h"
#include "WorldView.h"
#include "SectorView.h"
#include "Lang.h"
#include "Game.h"
#include "MathUtil.h"
#include "LuaEvent.h"

void Space::BodyNearFinder::Prepare()
{
	m_bodyDist.clear();

	for (Space::BodyIterator i = m_space->BodiesBegin(); i != m_space->BodiesEnd(); ++i)
		m_bodyDist.push_back(BodyDist((*i), (*i)->GetPositionRelTo(m_space->GetRootFrame()).Length()));

	std::sort(m_bodyDist.begin(), m_bodyDist.end());
}

void Space::BodyNearFinder::GetBodiesMaybeNear(const Body *b, double dist, BodyNearList &bodies) const
{
	GetBodiesMaybeNear(b->GetPositionRelTo(m_space->GetRootFrame()), dist, bodies);
}

void Space::BodyNearFinder::GetBodiesMaybeNear(const vector3d &pos, double dist, BodyNearList &bodies) const
{
	if (m_bodyDist.empty()) return;

	const double len = pos.Length();

	std::vector<BodyDist>::const_iterator min = std::lower_bound(m_bodyDist.begin(), m_bodyDist.end(), len-dist);
	std::vector<BodyDist>::const_iterator max = std::upper_bound(min, m_bodyDist.end(), len+dist);

	while (min != max) {
		bodies.push_back((*min).body);
		++min;
	}
}


Space::Space(Game *game)
	: m_game(game)
	, m_frameIndexValid(false)
	, m_bodyIndexValid(false)
	, m_sbodyIndexValid(false)
	, m_background(Pi::renderer, UNIVERSE_SEED)
	, m_bodyNearFinder(this)
#ifndef NDEBUG
	, m_processingFinalizationQueue(false)
#endif
{
	m_rootFrame.Reset(new Frame(0, Lang::SYSTEM));
	m_rootFrame->SetRadius(FLT_MAX);
}

Space::Space(Game *game, const SystemPath &path)
	: m_game(game)
	, m_frameIndexValid(false)
	, m_bodyIndexValid(false)
	, m_sbodyIndexValid(false)
	, m_background(Pi::renderer)
	, m_bodyNearFinder(this)
#ifndef NDEBUG
	, m_processingFinalizationQueue(false)
#endif
{
	m_starSystem = StarSystem::GetCached(path);
	m_background.Refresh(m_starSystem->GetSeed());

	// XXX set radius in constructor
	m_rootFrame.Reset(new Frame(0, Lang::SYSTEM));
	m_rootFrame->SetRadius(FLT_MAX);

	GenBody(m_starSystem->rootBody, m_rootFrame.Get());
	m_rootFrame->UpdateOrbitRails(m_game->GetTime(), m_game->GetTimeStep());

	//DebugDumpFrames();
}

Space::Space(Game *game, Serializer::Reader &rd)
	: m_game(game)
	, m_frameIndexValid(false)
	, m_bodyIndexValid(false)
	, m_sbodyIndexValid(false)
	, m_background(Pi::renderer)
	, m_bodyNearFinder(this)
#ifndef NDEBUG
	, m_processingFinalizationQueue(false)
#endif
{
	m_starSystem = StarSystem::Unserialize(rd);
	m_background.Refresh(m_starSystem->GetSeed());
	RebuildSystemBodyIndex();

	Serializer::Reader section = rd.RdSection("Frames");
	m_rootFrame.Reset(Frame::Unserialize(section, this, 0));
	RebuildFrameIndex();

	Uint32 nbodies = rd.Int32();
	for (Uint32 i = 0; i < nbodies; i++)
		m_bodies.push_back(Body::Unserialize(rd, this));
	RebuildBodyIndex();

	Frame::PostUnserializeFixup(m_rootFrame.Get(), this);
	for (BodyIterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
		(*i)->PostLoadFixup(this);
}

Space::~Space()
{
	UpdateBodies(); // make sure anything waiting to be removed gets removed before we go and kill everything else
	for (std::list<Body*>::iterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
		KillBody(*i);
	UpdateBodies();
}

void Space::Serialize(Serializer::Writer &wr)
{
	RebuildFrameIndex();
	RebuildBodyIndex();
	RebuildSystemBodyIndex();

	StarSystem::Serialize(wr, m_starSystem.Get());

	Serializer::Writer section;
	Frame::Serialize(section, m_rootFrame.Get(), this);
	wr.WrSection("Frames", section.GetData());

	wr.Int32(m_bodies.size());
	for (BodyIterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
		(*i)->Serialize(wr, this);
}

Frame *Space::GetFrameByIndex(Uint32 idx) const
{
	assert(m_frameIndexValid);
	assert(m_frameIndex.size() > idx);
	return m_frameIndex[idx];
}

Body *Space::GetBodyByIndex(Uint32 idx) const
{
	assert(m_bodyIndexValid);
	assert(m_bodyIndex.size() > idx);
	return m_bodyIndex[idx];
}

SystemBody *Space::GetSystemBodyByIndex(Uint32 idx) const
{
	assert(m_sbodyIndexValid);
	assert(m_sbodyIndex.size() > idx);
	return m_sbodyIndex[idx];
}

Uint32 Space::GetIndexForFrame(const Frame *frame) const
{
	assert(m_frameIndexValid);
	for (Uint32 i = 0; i < m_frameIndex.size(); i++)
		if (m_frameIndex[i] == frame) return i;
	assert(0);
	return Uint32(-1);
}

Uint32 Space::GetIndexForBody(const Body *body) const
{
	assert(m_bodyIndexValid);
	for (Uint32 i = 0; i < m_bodyIndex.size(); i++)
		if (m_bodyIndex[i] == body) return i;
	assert(0);
	return Uint32(-1);
}

Uint32 Space::GetIndexForSystemBody(const SystemBody *sbody) const
{
	assert(m_sbodyIndexValid);
	for (Uint32 i = 0; i < m_sbodyIndex.size(); i++)
		if (m_sbodyIndex[i] == sbody) return i;
	assert(0);
	return Uint32(-1);
}

void Space::AddFrameToIndex(Frame *frame)
{
	assert(frame);
	m_frameIndex.push_back(frame);
	for (Frame::ChildIterator it = frame->BeginChildren(); it != frame->EndChildren(); ++it)
		AddFrameToIndex(*it);
}

void Space::AddSystemBodyToIndex(SystemBody *sbody)
{
	assert(sbody);
	m_sbodyIndex.push_back(sbody);
	for (Uint32 i = 0; i < sbody->children.size(); i++)
		AddSystemBodyToIndex(sbody->children[i]);
}

void Space::RebuildFrameIndex()
{
	m_frameIndex.clear();
	m_frameIndex.push_back(0);

	if (m_rootFrame)
		AddFrameToIndex(m_rootFrame.Get());

	m_frameIndexValid = true;
}

void Space::RebuildBodyIndex()
{
	m_bodyIndex.clear();
	m_bodyIndex.push_back(0);

	for (BodyIterator i = m_bodies.begin(); i != m_bodies.end(); ++i) {
		m_bodyIndex.push_back(*i);
		// also index ships inside clouds
		// XXX we should not have to know about this. move indexing grunt work
		// down into the bodies?
		if ((*i)->IsType(Object::HYPERSPACECLOUD)) {
			Ship *s = static_cast<HyperspaceCloud*>(*i)->GetShip();
			if (s) m_bodyIndex.push_back(s);
		}
	}

	m_bodyIndexValid = true;
}

void Space::RebuildSystemBodyIndex()
{
	m_sbodyIndex.clear();
	m_sbodyIndex.push_back(0);

	if (m_starSystem)
		AddSystemBodyToIndex(m_starSystem->rootBody);

	m_sbodyIndexValid = true;
}

void Space::AddBody(Body *b)
{
	m_bodies.push_back(b);
}

void Space::RemoveBody(Body *b)
{
#ifndef NDEBUG
	assert(!m_processingFinalizationQueue);
#endif
	m_removeBodies.push_back(b);
}

void Space::KillBody(Body* b)
{
#ifndef NDEBUG
	assert(!m_processingFinalizationQueue);
#endif
	if (!b->IsDead()) {
		b->MarkDead();

		// player needs to stay alive so things like the death animation
		// (which uses a camera positioned relative to the player) can
		// continue to work. it will be cleaned up with the space is torn down
		// XXX this seems like the wrong way to do it. since its still "alive"
		// it still collides, moves, etc. better to just snapshot its position
		// elsewhere
		if (b != Pi::player)
			m_killBodies.push_back(b);
	}
}

vector3d Space::GetHyperspaceExitPoint(const SystemPath &source) const
{
	assert(m_starSystem);
	assert(source.IsSystemPath());

	const SystemPath &dest = m_starSystem->GetPath();

	Sector source_sec(source.sectorX, source.sectorY, source.sectorZ);
	Sector dest_sec(dest.sectorX, dest.sectorY, dest.sectorZ);

	Sector::System source_sys = source_sec.m_systems[source.systemIndex];
	Sector::System dest_sys = dest_sec.m_systems[dest.systemIndex];

	const vector3d sourcePos = vector3d(source_sys.p) + vector3d(source.sectorX, source.sectorY, source.sectorZ);
	const vector3d destPos = vector3d(dest_sys.p) + vector3d(dest.sectorX, dest.sectorY, dest.sectorZ);

	// find the first non-gravpoint. should be the primary star
	Body *primary = 0;
	for (BodyIterator i = BodiesBegin(); i != BodiesEnd(); ++i)
		if ((*i)->GetSystemBody()->type != SystemBody::TYPE_GRAVPOINT) {
			primary = *i;
			break;
		}
	assert(primary);

	// point along the line between source and dest, a reasonable distance
	// away based on the radius (don't want to end up inside black holes, and
	// then mix it up so that ships don't end up on top of each other
	vector3d pos = (sourcePos - destPos).Normalized() * (primary->GetSystemBody()->GetRadius()/AU+1.0)*11.0*AU*Pi::rng.Double(0.95,1.2) + MathUtil::RandomPointOnSphere(5.0,20.0)*1000.0;
	assert(pos.Length() > primary->GetSystemBody()->GetRadius());
	return pos + primary->GetPositionRelTo(GetRootFrame());
}

Body *Space::FindNearestTo(const Body *b, Object::Type t) const
{
	Body *nearest = 0;
	double dist = FLT_MAX;
	for (std::list<Body*>::const_iterator i = m_bodies.begin(); i != m_bodies.end(); ++i) {
		if ((*i)->IsDead()) continue;
		if ((*i)->IsType(t)) {
			double d = (*i)->GetPositionRelTo(b).Length();
			if (d < dist) {
				dist = d;
				nearest = *i;
			}
		}
	}
	return nearest;
}

Body *Space::FindBodyForPath(const SystemPath *path) const
{
	// it is a bit dumb that currentSystem is not part of Space...
	SystemBody *body = m_starSystem->GetBodyByPath(path);

	if (!body) return 0;

	for (BodyIterator i = m_bodies.begin(); i != m_bodies.end(); ++i) {
		if ((*i)->GetSystemBody() == body) return *i;
	}
	return 0;
}

static Frame *find_frame_with_sbody(Frame *f, const SystemBody *b)
{
	if (f->GetSystemBody() == b) return f;
	else {
		for (Frame::ChildIterator it = f->BeginChildren(); it != f->EndChildren(); ++it) {
			Frame *found = find_frame_with_sbody(*it, b);
			if (found) return found;
		}
	}
	return 0;
}

Frame *Space::GetFrameWithSystemBody(const SystemBody *b) const
{
	return find_frame_with_sbody(m_rootFrame.Get(), b);
}

static void RelocateStarportIfUnderwaterOrBuried(SystemBody *sbody, Frame *frame, Planet *planet, vector3d &pos, matrix3x3d &rot)
{
	const double radius = planet->GetSystemBody()->GetRadius();

	// suggested position
	rot = sbody->orbit.rotMatrix;
	pos = rot * vector3d(0,1,0);

	// Check if height varies too much around the starport center
	// by sampling 6 points around it. try upto 100 new positions randomly until a match is found
	// this is not guaranteed to find a match but greatly increases the chancessteroids which are not too steep.

	bool variationWithinLimits = true;
	double bestVariation = 1e10; // any high value
	matrix3x3d rotNotUnderwaterWithLeastVariation = rot;
	vector3d posNotUnderwaterWithLeastVariation = pos;
	const double heightVariationCheckThreshold = 0.008; // max variation to radius radius ratio to check for local slope, ganymede is around 0.01
	const double terrainHeightVariation = planet->GetGeoSphere()->GetMaxFeatureHeight(); //in radii

	//printf("%s: terrain height variation %f\n", sbody->name.c_str(), terrainHeightVariation);

	// 6 points are sampled around the starport center by adding/subtracting delta to to coords
	// points must stay within max height variation to be accepted
	//    1. delta should be chosen such that it a distance from the starport center that encloses landing pads for the largest starport
	//    2. maxSlope should be set so maxHeightVariation is less than the height of the landing pads
	const double delta = 20.0/radius; // in radii
	const double maxSlope = 0.2; // 0.0 to 1.0
	const double maxHeightVariation = maxSlope*delta*radius; // in m

	matrix3x3d rot_ = rot;
	vector3d pos_ = pos;

	bool manualRelocationIsEasy = !(planet->GetSystemBody()->type == SystemBody::TYPE_PLANET_ASTEROID || terrainHeightVariation > heightVariationCheckThreshold);

	// warn and leave it up to the user to relocate custom starports when it's easy to relocate manually, i.e. not on asteroids and other planets which are likely to have high variation in a lot of places
	const bool isRelocatableIfBuried = !(sbody->isCustomBody && manualRelocationIsEasy);

	bool isInitiallyUnderwater = false;
	bool initialVariationTooHigh = false;

	MTRand r(sbody->seed);

	for (int tries = 0; tries < 200; tries++) {
		variationWithinLimits = true;

		const double height = planet->GetTerrainHeight(pos_) - radius; // in m

		// check height at 6 points around the starport center stays within variation tolerances
		// GetHeight gives a varying height field in 3 dimensions.
		// Given it's smoothly varying it's fine to sample it in arbitary directions to get an idea of how sharply it varies
		double v[6];
		v[0] = fabs(planet->GetTerrainHeight(vector3d(pos_.x+delta, pos_.y, pos_.z))-radius-height);
		v[1] = fabs(planet->GetTerrainHeight(vector3d(pos_.x-delta, pos_.y, pos_.z))-radius-height);
		v[2] = fabs(planet->GetTerrainHeight(vector3d(pos_.x, pos_.y, pos_.z+delta))-radius-height);
		v[3] = fabs(planet->GetTerrainHeight(vector3d(pos_.x, pos_.y, pos_.z-delta))-radius-height);
		v[4] = fabs(planet->GetTerrainHeight(vector3d(pos_.x, pos_.y+delta, pos_.z))-radius-height);
		v[5] = fabs(planet->GetTerrainHeight(vector3d(pos_.x, pos_.y-delta, pos_.z))-radius-height);

		// break if variation for all points is within limits
		double variationMax = 0.0;
		for (int i = 0; i < 6; i++) {
			variationWithinLimits = variationWithinLimits && (v[i] < maxHeightVariation);
			variationMax = (v[i] > variationMax)? v[i]:variationMax;
		}

		// check if underwater
		const bool starportUnderwater = (height <= 0.0);

		//printf("%s: try no: %i, Match found: %i, best variation in previous results %f, variationMax this try: %f, maxHeightVariation: %f, Starport is underwater: %i\n",
		//	sbody->name.c_str(), tries, (variationWithinLimits && !starportUnderwater), bestVariation, variationMax, maxHeightVariation, starportUnderwater);

		if  (tries == 0) {
			isInitiallyUnderwater = starportUnderwater;
			initialVariationTooHigh = !variationWithinLimits;
		}

		if (!starportUnderwater && variationMax < bestVariation) {
			bestVariation = variationMax;
			posNotUnderwaterWithLeastVariation = pos_;
			rotNotUnderwaterWithLeastVariation = rot_;
		}

		if (variationWithinLimits && !starportUnderwater) break;

		// try new random position
		const double r2 = r.Double(); 	// function parameter evaluation order is implementation-dependent
		const double r1 = r.Double();	// can't put two rands in the same expression
		rot_ = matrix3x3d::RotateZ(2*M_PI*r1)
			* matrix3x3d::RotateY(2*M_PI*r2);
		pos_ = rot_ * vector3d(0,1,0);
	}

	if (isInitiallyUnderwater || (isRelocatableIfBuried && initialVariationTooHigh)) {
		pos = posNotUnderwaterWithLeastVariation;
		rot = rotNotUnderwaterWithLeastVariation;
	}

	if (sbody->isCustomBody) {
		SystemPath &p = sbody->path;
		if (initialVariationTooHigh) {
			if (isRelocatableIfBuried) {
				printf("Warning: Lua custom Systems definition: Surface starport has been automatically relocated. This is in order to place it on flatter ground to reduce the chance of landing pads being buried. This is not an error as such and you may attempt to move the starport to another location by changing latitude and longitude fields.\n      Surface starport name: %s, Body name: %s, In sector: x = %i, y = %i, z = %i.\n",
					sbody->name.c_str(), sbody->parent->name.c_str(), p.sectorX, p.sectorY, p.sectorZ);
			} else {
				printf("Warning: Lua custom Systems definition: Surface starport may have landing pads buried. The surface starport has not been automatically relocated as the planet appears smooth enough to manually relocate easily. This is not an error as such and you may attempt to move the starport to another location by changing latitude and longitude fields.\n      Surface starport name: %s, Body name: %s, In sector: x = %i, y = %i, z = %i.\n",
					sbody->name.c_str(), sbody->parent->name.c_str(), p.sectorX, p.sectorY, p.sectorZ);
			}
		}
		if (isInitiallyUnderwater) {
			fprintf(stderr, "Error: Lua custom Systems definition: Surface starport is underwater (height not greater than 0.0) and has been automatically relocated. Please move the starport to another location by changing latitude and longitude fields.\n      Surface starport name: %s, Body name: %s, In sector: x = %i, y = %i, z = %i.\n",
				sbody->name.c_str(), sbody->parent->name.c_str(), p.sectorX, p.sectorY, p.sectorZ);
		}
	}
}

static Frame *MakeFrameFor(SystemBody *sbody, Body *b, Frame *f)
{
	if (!sbody->parent) {
		if (b) b->SetFrame(f);
		f->SetBodies(sbody, b);
		return f;
	}

	if (sbody->type == SystemBody::TYPE_GRAVPOINT) {
		Frame *orbFrame = new Frame(f, sbody->name.c_str());
		orbFrame->SetBodies(sbody, b);
		orbFrame->SetRadius(sbody->GetMaxChildOrbitalDistance()*1.1);
		return orbFrame;
	}

	SystemBody::BodySuperType supertype = sbody->GetSuperType();

	if ((supertype == SystemBody::SUPERTYPE_GAS_GIANT) ||
	    (supertype == SystemBody::SUPERTYPE_ROCKY_PLANET)) {
		// for planets we want an non-rotating frame for a few radii
		// and a rotating frame with no radius to contain attached objects
		double frameRadius = std::max(4.0*sbody->GetRadius(), sbody->GetMaxChildOrbitalDistance()*1.05);
		Frame *orbFrame = new Frame(f, sbody->name.c_str(), Frame::FLAG_HAS_ROT);
		orbFrame->SetBodies(sbody, b);
		orbFrame->SetRadius(frameRadius);
		//printf("\t\t\t%s has frame size %.0fkm, body radius %.0fkm\n", sbody->name.c_str(),
		//	(frameRadius ? frameRadius : 10*sbody->GetRadius())*0.001f,
		//	sbody->GetRadius()*0.001f);

		assert(sbody->rotationPeriod != 0);
		Frame *rotFrame = new Frame(orbFrame, sbody->name.c_str(), Frame::FLAG_ROTATING);
		rotFrame->SetBodies(sbody, b);

		// rotating frame has atmosphere radius or feature height, whichever is larger
		rotFrame->SetRadius(b->GetPhysRadius());

		matrix3x3d rotMatrix = matrix3x3d::RotateX(sbody->axialTilt.ToDouble());
		double angSpeed = 2.0*M_PI/sbody->GetRotationPeriod();
		rotFrame->SetAngSpeed(angSpeed);

		if (sbody->rotationalPhaseAtStart != fixed(0))
			rotMatrix = rotMatrix * matrix3x3d::RotateY(sbody->rotationalPhaseAtStart.ToDouble());
		rotFrame->SetOrient(rotMatrix);

		b->SetFrame(rotFrame);
		return orbFrame;
	}
	else if (supertype == SystemBody::SUPERTYPE_STAR) {
		// stars want a single small non-rotating frame
		// bigger than it's furtherest orbiting body.
		// if there are no orbiting bodies use a frame of several radii.
		Frame *orbFrame = new Frame(f, sbody->name.c_str());
		orbFrame->SetBodies(sbody, b);
		orbFrame->SetRadius(std::max(10.0*sbody->GetRadius(), sbody->GetMaxChildOrbitalDistance()*1.1));
		b->SetFrame(orbFrame);
		return orbFrame;
	}
	else if (sbody->type == SystemBody::TYPE_STARPORT_ORBITAL) {
		// space stations want non-rotating frame to some distance
		// and a zero-size rotating frame
		Frame *orbFrame = new Frame(f, sbody->name.c_str(), Frame::FLAG_HAS_ROT);
		orbFrame->SetBodies(sbody, b);
//		orbFrame->SetRadius(10*sbody->GetRadius());
		orbFrame->SetRadius(20000.0);				// 4x standard parking radius
		b->SetFrame(orbFrame);
		return orbFrame;

//		assert(sbody->rotationPeriod != 0);
//		rotFrame = new Frame(orbFrame, sbody->name.c_str(), Frame::FLAG_ROTATING);
//		rotFrame->SetBodies(sbody, b);
//		rotFrame->SetRadius(0.0);
//		rotFrame->SetAngVelocity(vector3d(0.0,double(static_cast<SpaceStation*>(b)->GetDesiredAngVel()),0.0));
//		b->SetFrame(rotFrame);

	} else if (sbody->type == SystemBody::TYPE_STARPORT_SURFACE) {
		// just put body into rotating frame of planet, not in its own frame
		// (because collisions only happen between objects in same frame,
		// and we want collisions on starport and on planet itself)
		Frame *rotFrame = f->GetRotFrame();
		b->SetFrame(rotFrame);
		assert(rotFrame->IsRotFrame());
		assert(rotFrame->GetBody()->IsType(Object::PLANET));
		matrix3x3d rot;
		vector3d pos;
		Planet *planet = static_cast<Planet*>(rotFrame->GetBody());
		RelocateStarportIfUnderwaterOrBuried(sbody, rotFrame, planet, pos, rot);
		sbody->orbit.rotMatrix = rot;
		b->SetPosition(pos * planet->GetTerrainHeight(pos));
		b->SetOrient(rot);
		return rotFrame;
	} else {
		assert(0);
	}
	return NULL;
}

void Space::GenBody(SystemBody *sbody, Frame *f)
{
	Body *b = 0;

	if (sbody->type != SystemBody::TYPE_GRAVPOINT) {
		if (sbody->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
			Star *star = new Star(sbody);
			b = star;
		} else if ((sbody->type == SystemBody::TYPE_STARPORT_ORBITAL) ||
		           (sbody->type == SystemBody::TYPE_STARPORT_SURFACE)) {
			SpaceStation *ss = new SpaceStation(sbody);
			b = ss;
		} else {
			Planet *planet = new Planet(sbody);
			b = planet;
		}
		b->SetLabel(sbody->name.c_str());
		b->SetPosition(vector3d(0,0,0));
		AddBody(b);
	}
	f = MakeFrameFor(sbody, b, f);

	for (std::vector<SystemBody*>::iterator i = sbody->children.begin(); i != sbody->children.end(); ++i) {
		GenBody(*i, f);
	}
}

static bool OnCollision(Object *o1, Object *o2, CollisionContact *c, double relativeVel)
{
	Body *pb1 = static_cast<Body*>(o1);
	Body *pb2 = static_cast<Body*>(o2);
	/* Not always a Body (could be CityOnPlanet, which is a nasty exception I should eradicate) */
	if (o1->IsType(Object::BODY)) {
		if (pb1 && !pb1->OnCollision(o2, c->geomFlag, relativeVel)) return false;
	}
	if (o2->IsType(Object::BODY)) {
		if (pb2 && !pb2->OnCollision(o1, c->geomFlag, relativeVel)) return false;
	}
	return true;
}

static void hitCallback(CollisionContact *c)
{
	//printf("OUCH! %x (depth %f)\n", SDL_GetTicks(), c->depth);

	Object *po1 = static_cast<Object*>(c->userData1);
	Object *po2 = static_cast<Object*>(c->userData2);

	const bool po1_isDynBody = po1->IsType(Object::DYNAMICBODY);
	const bool po2_isDynBody = po2->IsType(Object::DYNAMICBODY);
	// collision response
	assert(po1_isDynBody || po2_isDynBody);

	if (po1_isDynBody && po2_isDynBody) {
		DynamicBody *b1 = static_cast<DynamicBody*>(po1);
		DynamicBody *b2 = static_cast<DynamicBody*>(po2);
		const vector3d linVel1 = b1->GetVelocity();
		const vector3d linVel2 = b2->GetVelocity();
		const vector3d angVel1 = b1->GetAngVelocity();
		const vector3d angVel2 = b2->GetAngVelocity();

		const double coeff_rest = 0.5;
		// step back
//		mover->UndoTimestep();

		const double invMass1 = 1.0 / b1->GetMass();
		const double invMass2 = 1.0 / b2->GetMass();
		const vector3d hitPos1 = c->pos - b1->GetPosition();
		const vector3d hitPos2 = c->pos - b2->GetPosition();
		const vector3d hitVel1 = linVel1 + angVel1.Cross(hitPos1);
		const vector3d hitVel2 = linVel2 + angVel2.Cross(hitPos2);
		const double relVel = (hitVel1 - hitVel2).Dot(c->normal);
		// moving away so no collision
		if (relVel > 0) return;
		if (!OnCollision(po1, po2, c, -relVel)) return;
		const double invAngInert1 = 1.0 / b1->GetAngularInertia();
		const double invAngInert2 = 1.0 / b2->GetAngularInertia();
		const double numerator = -(1.0 + coeff_rest) * relVel;
		const double term1 = invMass1;
		const double term2 = invMass2;
		const double term3 = c->normal.Dot((hitPos1.Cross(c->normal)*invAngInert1).Cross(hitPos1));
		const double term4 = c->normal.Dot((hitPos2.Cross(c->normal)*invAngInert2).Cross(hitPos2));

		const double j = numerator / (term1 + term2 + term3 + term4);
		const vector3d force = j * c->normal;

		b1->SetVelocity(linVel1 + force*invMass1);
		b1->SetAngVelocity(angVel1 + hitPos1.Cross(force)*invAngInert1);
		b2->SetVelocity(linVel2 - force*invMass2);
		b2->SetAngVelocity(angVel2 - hitPos2.Cross(force)*invAngInert2);
	} else {
		// one body is static
		vector3d hitNormal;
		DynamicBody *mover;

		if (po1_isDynBody) {
			mover = static_cast<DynamicBody*>(po1);
			hitNormal = c->normal;
		} else {
			mover = static_cast<DynamicBody*>(po2);
			hitNormal = -c->normal;
		}

		const double coeff_rest = 0.5;
		const vector3d linVel1 = mover->GetVelocity();
		const vector3d angVel1 = mover->GetAngVelocity();

		// step back
//		mover->UndoTimestep();

		const double invMass1 = 1.0 / mover->GetMass();
		const vector3d hitPos1 = c->pos - mover->GetPosition();
		const vector3d hitVel1 = linVel1 + angVel1.Cross(hitPos1);
		const double relVel = hitVel1.Dot(c->normal);
		// moving away so no collision
		if (relVel > 0 && !c->geomFlag) return;
		if (!OnCollision(po1, po2, c, -relVel)) return;
		const double invAngInert = 1.0 / mover->GetAngularInertia();
		const double numerator = -(1.0 + coeff_rest) * relVel;
		const double term1 = invMass1;
		const double term3 = c->normal.Dot((hitPos1.Cross(c->normal)*invAngInert).Cross(hitPos1));

		const double j = numerator / (term1 + term3);
		const vector3d force = j * c->normal;

		mover->SetVelocity(linVel1 + force*invMass1);
		mover->SetAngVelocity(angVel1 + hitPos1.Cross(force)*invAngInert);
	}
}

// temporary one-point version
static void CollideWithTerrain(Body *body)
{
	if (!body->IsType(Object::DYNAMICBODY)) return;
	DynamicBody *dynBody = static_cast<DynamicBody*>(body);
	if (!dynBody->IsMoving()) return;

	Frame *f = body->GetFrame();
	if (!f || !f->GetBody() || f != f->GetBody()->GetFrame()) return;
	if (!f->GetBody()->IsType(Object::TERRAINBODY)) return;
	TerrainBody *terrain = static_cast<TerrainBody*>(f->GetBody());

	const Aabb &aabb = dynBody->GetAabb();
	double altitude = body->GetPosition().Length() + aabb.min.y;
	if (altitude >= terrain->GetMaxFeatureRadius()) return;

	double terrHeight = terrain->GetTerrainHeight(body->GetPosition().Normalized());
	if (altitude >= terrHeight) return;
	
	CollisionContact c;
	c.pos = body->GetPosition();
	c.normal = c.pos.Normalized();
	c.depth = terrHeight - altitude;
	c.userData1 = static_cast<void*>(body);
	c.userData2 = static_cast<void*>(f->GetBody());
	hitCallback(&c);
}

void Space::CollideFrame(Frame *f)
{
	f->GetCollisionSpace()->Collide(&hitCallback);
	for (Frame::ChildIterator it = f->BeginChildren(); it != f->EndChildren(); ++it)
		CollideFrame(*it);
}

void Space::TimeStep(float step)
{
	m_frameIndexValid = m_bodyIndexValid = m_sbodyIndexValid = false;

	// XXX does not need to be done this often
	CollideFrame(m_rootFrame.Get());
	for (BodyIterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
		CollideWithTerrain(*i);

	// update frames of reference
	for (BodyIterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
		(*i)->UpdateFrame();

	// AI acts here, then move all bodies and frames
	for (BodyIterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
		(*i)->StaticUpdate(step);

	m_rootFrame->UpdateOrbitRails(m_game->GetTime(), m_game->GetTimeStep());

	for (BodyIterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
		(*i)->TimeStepUpdate(step);

	// XXX don't emit events in hyperspace. this is mostly to maintain the
	// status quo. in particular without this onEnterSystem will fire in the
	// frame immediately before the player leaves hyperspace and the system is
	// invalid when Lua goes and queries for it. we need to consider whether
	// there's anything useful that can be done with events in hyperspace
	if (m_starSystem) {
		LuaEvent::Emit();
		Pi::luaTimer->Tick();
	}

	UpdateBodies();

	m_bodyNearFinder.Prepare();
}

void Space::UpdateBodies()
{
#ifndef NDEBUG
	m_processingFinalizationQueue = true;
#endif

	for (BodyIterator b = m_removeBodies.begin(); b != m_removeBodies.end(); ++b) {
		(*b)->SetFrame(0);
		for (BodyIterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
			(*i)->NotifyRemoved(*b);
		m_bodies.remove(*b);
	}
	m_removeBodies.clear();

	for (BodyIterator b = m_killBodies.begin(); b != m_killBodies.end(); ++b) {
		for (BodyIterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
			(*i)->NotifyRemoved(*b);
		m_bodies.remove(*b);
		delete *b;
	}
	m_killBodies.clear();

#ifndef NDEBUG
	m_processingFinalizationQueue = false;
#endif
}

static char space[256];

static void DebugDumpFrame(Frame *f, unsigned int indent)
{
	printf("%.*s%p (%s)", indent, space, f, f->GetLabel().c_str());
	if (f->GetParent())
		printf(" parent %p (%s)", f->GetParent(), f->GetParent()->GetLabel().c_str());
	if (f->GetBody())
		printf(" body %p (%s)", f->GetBody(), f->GetBody()->GetLabel().c_str());
	if (Body *b = f->GetBody())
		printf(" bodyFor %p (%s)", b, b->GetLabel().c_str());
	printf(" distance %f radius %f", f->GetPosition().Length(), f->GetRadius());
	printf("%s\n", f->IsRotFrame() ? " [rotating]" : "");

	for (Frame::ChildIterator it = f->BeginChildren(); it != f->EndChildren(); ++it)
		DebugDumpFrame(*it, indent+2);
}

void Space::DebugDumpFrames()
{
	memset(space, ' ', sizeof(space));

	printf("Frame structure for '%s':\n", m_starSystem->GetName().c_str());
	DebugDumpFrame(m_rootFrame.Get(), 2);
}
