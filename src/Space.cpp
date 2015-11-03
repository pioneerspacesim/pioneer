// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "Space.h"
#include "Body.h"
#include "Frame.h"
#include "Star.h"
#include "Planet.h"
#include "CityOnPlanet.h"
#include <algorithm>
#include <functional>
#include "Pi.h"
#include "Player.h"
#include "galaxy/Galaxy.h"
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

//#define DEBUG_CACHE

void Space::BodyNearFinder::Prepare()
{
	m_bodyDist.clear();

	for (Body* b : m_space->GetBodies())
		m_bodyDist.push_back(BodyDist(b, b->GetPositionRelTo(m_space->GetRootFrame()).Length()));

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

Space::Space(Game *game, RefCountedPtr<Galaxy> galaxy, Space* oldSpace)
	: m_starSystemCache(oldSpace ? oldSpace->m_starSystemCache : galaxy->NewStarSystemSlaveCache())
	, m_game(game)
	, m_frameIndexValid(false)
	, m_bodyIndexValid(false)
	, m_sbodyIndexValid(false)
	, m_bodyNearFinder(this)
#ifndef NDEBUG
	, m_processingFinalizationQueue(false)
#endif
{
	m_background.reset(new Background::Container(Pi::renderer, Pi::rng));

	m_rootFrame.reset(new Frame(0, Lang::SYSTEM));
	m_rootFrame->SetRadius(FLT_MAX);

	GenSectorCache(galaxy, &game->GetHyperspaceDest());
}

Space::Space(Game *game, RefCountedPtr<Galaxy> galaxy, const SystemPath &path, Space* oldSpace)
	: m_starSystemCache(oldSpace ? oldSpace->m_starSystemCache : galaxy->NewStarSystemSlaveCache())
	, m_starSystem(galaxy->GetStarSystem(path))
	, m_game(game)
	, m_frameIndexValid(false)
	, m_bodyIndexValid(false)
	, m_sbodyIndexValid(false)
	, m_bodyNearFinder(this)
#ifndef NDEBUG
	, m_processingFinalizationQueue(false)
#endif
{
	Uint32 _init[5] = { path.systemIndex, Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rand(_init, 5);
	m_background.reset(new Background::Container(Pi::renderer, rand));

	CityOnPlanet::SetCityModelPatterns(m_starSystem->GetPath());

	// XXX set radius in constructor
	m_rootFrame.reset(new Frame(0, Lang::SYSTEM));
	m_rootFrame->SetRadius(FLT_MAX);

	std::vector<vector3d> positionAccumulator;
	GenBody(m_game->GetTime(), m_starSystem->GetRootBody().Get(), m_rootFrame.get(), positionAccumulator);
	m_rootFrame->UpdateOrbitRails(m_game->GetTime(), m_game->GetTimeStep());

	GenSectorCache(galaxy, &path);

	//DebugDumpFrames();
}

Space::Space(Game *game, RefCountedPtr<Galaxy> galaxy, const Json::Value &jsonObj, double at_time)
	: m_starSystemCache(galaxy->NewStarSystemSlaveCache())
	, m_game(game)
	, m_frameIndexValid(false)
	, m_bodyIndexValid(false)
	, m_sbodyIndexValid(false)
	, m_bodyNearFinder(this)
#ifndef NDEBUG
	, m_processingFinalizationQueue(false)
#endif
{
	if (!jsonObj.isMember("space")) throw SavedGameCorruptException();
	Json::Value spaceObj = jsonObj["space"];

	m_starSystem = StarSystem::FromJson(galaxy, spaceObj);

	const SystemPath &path = m_starSystem->GetPath();
	Uint32 _init[5] = { path.systemIndex, Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rand(_init, 5);
	m_background.reset(new Background::Container(Pi::renderer, rand));

	RebuildSystemBodyIndex();

	CityOnPlanet::SetCityModelPatterns(m_starSystem->GetPath());

	m_rootFrame.reset(Frame::FromJson(spaceObj, this, 0, at_time));
	RebuildFrameIndex();

	if (!spaceObj.isMember("bodies")) throw SavedGameCorruptException();
	Json::Value bodyArray = spaceObj["bodies"];
	if (!bodyArray.isArray()) throw SavedGameCorruptException();
	for (Uint32 i = 0; i < bodyArray.size(); i++)
		m_bodies.push_back(Body::FromJson(bodyArray[i], this));
	RebuildBodyIndex();

	Frame::PostUnserializeFixup(m_rootFrame.get(), this);
	for (Body* b : m_bodies)
		b->PostLoadFixup(this);

	GenSectorCache(galaxy, &path);
}

Space::~Space()
{
	UpdateBodies(); // make sure anything waiting to be removed gets removed before we go and kill everything else
	for (std::list<Body*>::iterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
		KillBody(*i);
	UpdateBodies();
}

void Space::RefreshBackground()
{
	const SystemPath &path = m_starSystem->GetPath();
	Uint32 _init[5] = { path.systemIndex, Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rand(_init, 5);
	m_background.reset(new Background::Container(Pi::renderer, rand));
}

void Space::ToJson(Json::Value &jsonObj)
{
	RebuildFrameIndex();
	RebuildBodyIndex();
	RebuildSystemBodyIndex();

	Json::Value spaceObj(Json::objectValue); // Create JSON object to contain space data (all the bodies and things).

	StarSystem::ToJson(spaceObj, m_starSystem.Get());

	Frame::ToJson(spaceObj, m_rootFrame.get(), this);

	Json::Value bodyArray(Json::arrayValue); // Create JSON array to contain body data.
	for (Body* b : m_bodies)
	{
		Json::Value bodyArrayEl(Json::objectValue); // Create JSON object to contain body.
		b->ToJson(bodyArrayEl, this);
		bodyArray.append(bodyArrayEl); // Append body object to array.
	}
	spaceObj["bodies"] = bodyArray; // Add body array to space object.

	jsonObj["space"] = spaceObj; // Add space object to supplied object.
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
	for (Frame* kid : frame->GetChildren())
		AddFrameToIndex(kid);
}

void Space::AddSystemBodyToIndex(SystemBody *sbody)
{
	assert(sbody);
	m_sbodyIndex.push_back(sbody);
	for (Uint32 i = 0; i < sbody->GetNumChildren(); i++)
		AddSystemBodyToIndex(sbody->GetChildren()[i]);
}

void Space::RebuildFrameIndex()
{
	m_frameIndex.clear();
	m_frameIndex.push_back(0);

	if (m_rootFrame)
		AddFrameToIndex(m_rootFrame.get());

	m_frameIndexValid = true;
}

void Space::RebuildBodyIndex()
{
	m_bodyIndex.clear();
	m_bodyIndex.push_back(0);

	for (Body* b : m_bodies) {
		m_bodyIndex.push_back(b);
		// also index ships inside clouds
		// XXX we should not have to know about this. move indexing grunt work
		// down into the bodies?
		if (b->IsType(Object::HYPERSPACECLOUD)) {
			Ship *s = static_cast<HyperspaceCloud*>(b)->GetShip();
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
		AddSystemBodyToIndex(m_starSystem->GetRootBody().Get());

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

vector3d Space::GetHyperspaceExitPoint(const SystemPath &source, const SystemPath &dest) const
{
	assert(m_starSystem);
	assert(source.IsSystemPath());

	assert(dest.IsSameSystem(m_starSystem->GetPath()));

	RefCountedPtr<const Sector> source_sec = m_sectorCache->GetCached(source);
	RefCountedPtr<const Sector> dest_sec = m_sectorCache->GetCached(dest);

	Sector::System source_sys = source_sec->m_systems[source.systemIndex];
	Sector::System dest_sys = dest_sec->m_systems[dest.systemIndex];

	const vector3d sourcePos = vector3d(source_sys.GetPosition()) + vector3d(source.sectorX, source.sectorY, source.sectorZ);
	const vector3d destPos = vector3d(dest_sys.GetPosition()) + vector3d(dest.sectorX, dest.sectorY, dest.sectorZ);

	Body *primary = 0;
	if (dest.IsBodyPath()) {
		assert(dest.bodyIndex < m_starSystem->GetNumBodies());
		primary = FindBodyForPath(&dest);
		while (primary && primary->GetSystemBody()->GetSuperType() != SystemBody::SUPERTYPE_STAR) {
			SystemBody* parent = primary->GetSystemBody()->GetParent();
			primary = parent ? FindBodyForPath(&parent->GetPath()) : 0;
		}
	}
	if (!primary) {
		// find the first non-gravpoint. should be the primary star
		for (Body* b : GetBodies())
			if (b->GetSystemBody()->GetType() != SystemBody::TYPE_GRAVPOINT) {
				primary = b;
				break;
			}
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

	for (Body* b : m_bodies) {
		if (b->GetSystemBody() == body) return b;
	}
	return 0;
}

static Frame *find_frame_with_sbody(Frame *f, const SystemBody *b)
{
	if (f->GetSystemBody() == b) return f;
	else {
		for (Frame* kid : f->GetChildren()) {
			Frame *found = find_frame_with_sbody(kid, b);
			if (found) return found;
		}
	}
	return 0;
}

Frame *Space::GetFrameWithSystemBody(const SystemBody *b) const
{
	return find_frame_with_sbody(m_rootFrame.get(), b);
}

static void RelocateStarportIfNecessary(SystemBody *sbody, Frame *frame, Planet *planet, vector3d &pos, matrix3x3d &rot, const std::vector<vector3d> &prevPositions)
{
	const double radius = planet->GetSystemBody()->GetRadius();

	// suggested position
	rot = sbody->GetOrbit().GetPlane();
	pos = rot * vector3d(0,1,0);

	// Check if height varies too much around the starport center
	// by sampling 6 points around it. try upto 100 new positions randomly until a match is found
	// this is not guaranteed to find a match but greatly increases the chancessteroids which are not too steep.

	bool variationWithinLimits = true;
	double bestVariation = 1e10; // any high value
	matrix3x3d rotNotUnderwaterWithLeastVariation = rot;
	vector3d posNotUnderwaterWithLeastVariation = pos;
	const double heightVariationCheckThreshold = 0.008; // max variation to radius radius ratio to check for local slope, ganymede is around 0.01
	const double terrainHeightVariation = planet->GetMaxFeatureRadius(); //in radii

	//Output("%s: terrain height variation %f\n", sbody->name.c_str(), terrainHeightVariation);

	// 6 points are sampled around the starport center by adding/subtracting delta to to coords
	// points must stay within max height variation to be accepted
	//    1. delta should be chosen such that it a distance from the starport center that encloses landing pads for the largest starport
	//    2. maxSlope should be set so maxHeightVariation is less than the height of the landing pads
	const double delta = 20.0/radius; // in radii
	const double maxSlope = 0.2; // 0.0 to 1.0
	const double maxHeightVariation = maxSlope*delta*radius; // in m

	matrix3x3d rot_ = rot;
	vector3d pos_ = pos;

	const bool manualRelocationIsEasy = !(planet->GetSystemBody()->GetType() == SystemBody::TYPE_PLANET_ASTEROID || terrainHeightVariation > heightVariationCheckThreshold);

	// warn and leave it up to the user to relocate custom starports when it's easy to relocate manually, i.e. not on asteroids and other planets which are likely to have high variation in a lot of places
	const bool isRelocatableIfBuried = !(sbody->IsCustomBody() && manualRelocationIsEasy);

	bool isInitiallyUnderwater = false;
	bool initialVariationTooHigh = false;

	Random r(sbody->GetSeed());

	for (int tries = 0; tries < 200; tries++) 
	{
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

		//Output("%s: try no: %i, Match found: %i, best variation in previous results %f, variationMax this try: %f, maxHeightVariation: %f, Starport is underwater: %i\n",
		//	sbody->name.c_str(), tries, (variationWithinLimits && !starportUnderwater), bestVariation, variationMax, maxHeightVariation, starportUnderwater);

		bool tooCloseToOther = false;
		for (vector3d oldPos : prevPositions)
		{
			// is the distance between points less than the delta distance?	
			if ((pos_ - oldPos).LengthSqr() < (delta*delta)) {
				tooCloseToOther = true; // then we're too close so try again
				break;
			}
		}

		if  (tries == 0) {
			isInitiallyUnderwater = starportUnderwater;
			initialVariationTooHigh = !variationWithinLimits;
		}

		if (!starportUnderwater && variationMax < bestVariation) {
			bestVariation = variationMax;
			posNotUnderwaterWithLeastVariation = pos_;
			rotNotUnderwaterWithLeastVariation = rot_;
		}

		if (variationWithinLimits && !starportUnderwater && !tooCloseToOther) 
			break;

		// try new random position
		const double r3 = r.Double();
		const double r2 = r.Double(); 	// function parameter evaluation order is implementation-dependent
		const double r1 = r.Double();	// can't put two rands in the same expression
		rot_ = matrix3x3d::RotateZ(2.0*M_PI*r1)
			* matrix3x3d::RotateY(2.0*M_PI*r2)
			* matrix3x3d::RotateX(2.0*M_PI*r3);
		pos_ = rot_ * vector3d(0,1,0);
	}

	if (isInitiallyUnderwater || (isRelocatableIfBuried && initialVariationTooHigh)) {
		pos = posNotUnderwaterWithLeastVariation;
		rot = rotNotUnderwaterWithLeastVariation;
	}

	if (sbody->IsCustomBody()) {
		const SystemPath &p = sbody->GetPath();
		if (initialVariationTooHigh) {
			if (isRelocatableIfBuried) {
				Output("Warning: Lua custom Systems definition: Surface starport has been automatically relocated. This is in order to place it on flatter ground to reduce the chance of landing pads being buried. This is not an error as such and you may attempt to move the starport to another location by changing latitude and longitude fields.\n      Surface starport name: %s, Body name: %s, In sector: x = %i, y = %i, z = %i.\n",
					sbody->GetName().c_str(), sbody->GetParent()->GetName().c_str(), p.sectorX, p.sectorY, p.sectorZ);
			} else {
				Output("Warning: Lua custom Systems definition: Surface starport may have landing pads buried. The surface starport has not been automatically relocated as the planet appears smooth enough to manually relocate easily. This is not an error as such and you may attempt to move the starport to another location by changing latitude and longitude fields.\n      Surface starport name: %s, Body name: %s, In sector: x = %i, y = %i, z = %i.\n",
					sbody->GetName().c_str(), sbody->GetParent()->GetName().c_str(), p.sectorX, p.sectorY, p.sectorZ);
			}
		}
		if (isInitiallyUnderwater) {
			Output("Error: Lua custom Systems definition: Surface starport is underwater (height not greater than 0.0) and has been automatically relocated. Please move the starport to another location by changing latitude and longitude fields.\n      Surface starport name: %s, Body name: %s, In sector: x = %i, y = %i, z = %i.\n",
				sbody->GetName().c_str(), sbody->GetParent()->GetName().c_str(), p.sectorX, p.sectorY, p.sectorZ);
		}
	}
}

static Frame *MakeFrameFor(const double at_time, SystemBody *sbody, Body *b, Frame *f, std::vector<vector3d> &prevPositions)
{
	if (!sbody->GetParent()) {
		if (b) b->SetFrame(f);
		f->SetBodies(sbody, b);
		return f;
	}

	if (sbody->GetType() == SystemBody::TYPE_GRAVPOINT) {
		Frame *orbFrame = new Frame(f, sbody->GetName().c_str());
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
		Frame *orbFrame = new Frame(f, sbody->GetName().c_str(), Frame::FLAG_HAS_ROT);
		orbFrame->SetBodies(sbody, b);
		orbFrame->SetRadius(frameRadius);
		//Output("\t\t\t%s has frame size %.0fkm, body radius %.0fkm\n", sbody->name.c_str(),
		//	(frameRadius ? frameRadius : 10*sbody->GetRadius())*0.001f,
		//	sbody->GetRadius()*0.001f);

		assert(sbody->IsRotating() != 0);
		Frame *rotFrame = new Frame(orbFrame, sbody->GetName().c_str(), Frame::FLAG_ROTATING);
		rotFrame->SetBodies(sbody, b);

		// rotating frame has atmosphere radius or feature height, whichever is larger
		rotFrame->SetRadius(b->GetPhysRadius());

		matrix3x3d rotMatrix = matrix3x3d::RotateX(sbody->GetAxialTilt());
		double angSpeed = 2.0*M_PI/sbody->GetRotationPeriod();
		rotFrame->SetAngSpeed(angSpeed);

		if (sbody->HasRotationPhase())
			rotMatrix = rotMatrix * matrix3x3d::RotateY(sbody->GetRotationPhaseAtStart());
		rotFrame->SetInitialOrient(rotMatrix, at_time);

		b->SetFrame(rotFrame);
		return orbFrame;
	}
	else if (supertype == SystemBody::SUPERTYPE_STAR) {
		// stars want a single small non-rotating frame
		// bigger than it's furtherest orbiting body.
		// if there are no orbiting bodies use a frame of several radii.
		Frame *orbFrame = new Frame(f, sbody->GetName().c_str());
		orbFrame->SetBodies(sbody, b);
		double frameRadius = std::max(10.0*sbody->GetRadius(), sbody->GetMaxChildOrbitalDistance()*1.1);
		// Respect the frame of other stars in the multi-star system. We still make sure that the frame ends outside
		// the body. For a minimum separation of 1.236 radii, nothing will overlap (see StarSystem::StarSystem()).
		if (sbody->GetParent() && frameRadius > AU * 0.11 * sbody->GetOrbMin())
			frameRadius = std::max(1.1*sbody->GetRadius(), AU * 0.11 * sbody->GetOrbMin());
		orbFrame->SetRadius(frameRadius);
		b->SetFrame(orbFrame);
		return orbFrame;
	}
	else if (sbody->GetType() == SystemBody::TYPE_STARPORT_ORBITAL) {
		// space stations want non-rotating frame to some distance
		Frame *orbFrame = new Frame(f, sbody->GetName().c_str());
		orbFrame->SetBodies(sbody, b);
//		orbFrame->SetRadius(10*sbody->GetRadius());
		orbFrame->SetRadius(20000.0);				// 4x standard parking radius
		b->SetFrame(orbFrame);
		return orbFrame;

	} else if (sbody->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
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
		RelocateStarportIfNecessary(sbody, rotFrame, planet, pos, rot, prevPositions);
		sbody->SetOrbitPlane(rot);
		b->SetPosition(pos * planet->GetTerrainHeight(pos));
		b->SetOrient(rot);
		// accumulate for testing against
		prevPositions.push_back(pos);
		return rotFrame;
	} else {
		assert(0);
	}
	return 0;
}

// used to define a cube centred on your current location
static const int sectorRadius = 5;

// sort using a custom function object
class SectorDistanceSort {
public:
	bool operator()(const SystemPath &a, const SystemPath &b)
	{
		const float dist_a = vector3f(here.sectorX - a.sectorX, here.sectorY - a.sectorY, here.sectorZ - a.sectorZ).LengthSqr();
		const float dist_b = vector3f(here.sectorX - b.sectorX, here.sectorY - b.sectorY, here.sectorZ - b.sectorZ).LengthSqr();
		return dist_a < dist_b;
	}
	SectorDistanceSort( const SystemPath* centre )
		: here(centre)
	{}
private:
	SectorDistanceSort() {}
	SystemPath here;
};

void Space::GenSectorCache(RefCountedPtr<Galaxy> galaxy, const SystemPath* here)
{
	PROFILE_SCOPED()

	// current location
	if (!here) {
		if (!m_starSystem.Valid())
			return;
		here = &m_starSystem->GetPath();
	}
	const int here_x = here->sectorX;
	const int here_y = here->sectorY;
	const int here_z = here->sectorZ;

	SectorCache::PathVector paths;
	// build all of the possible paths we'll need to build sectors for
	for (int x = here_x-sectorRadius; x <= here_x+sectorRadius; x++) {
		for (int y = here_y-sectorRadius; y <= here_y+sectorRadius; y++) {
			for (int z = here_z-sectorRadius; z <= here_z+sectorRadius; z++) {
				SystemPath path(x, y, z);
				paths.push_back(path);
			}
		}
	}
	// sort them so that those closest to the "here" path are processed first
	SectorDistanceSort SDS(here);
	std::sort(paths.begin(), paths.end(), SDS);
	m_sectorCache = galaxy->NewSectorSlaveCache();
	const SystemPath& center(*here);
	m_sectorCache->FillCache(paths, [this,center]() { UpdateStarSystemCache(&center); });
}

static bool WithinBox(const SystemPath &here, const int Xmin, const int Xmax, const int Ymin, const int Ymax, const int Zmin, const int Zmax) {
	PROFILE_SCOPED()
	if(here.sectorX >= Xmin && here.sectorX <= Xmax) {
		if(here.sectorY >= Ymin && here.sectorY <= Ymax) {
			if(here.sectorZ >= Zmin && here.sectorZ <= Zmax) {
				return true;
			}
		}
	}
	return false;
}

void Space::UpdateStarSystemCache(const SystemPath* here)
{
	PROFILE_SCOPED()

	// current location
	if (!here) {
		if (!m_starSystem.Valid())
			return;
		here = &m_starSystem->GetPath();
	}
	const int here_x = here->sectorX;
	const int here_y = here->sectorY;
	const int here_z = here->sectorZ;

	// we're going to use these to determine if our StarSystems are within a range that we'll keep for later use
	static const int survivorRadius = sectorRadius*3;

	// min/max box limits
	const int xmin = here->sectorX-survivorRadius;
	const int xmax = here->sectorX+survivorRadius;
	const int ymin = here->sectorY-survivorRadius;
	const int ymax = here->sectorY+survivorRadius;
	const int zmin = here->sectorZ-survivorRadius;
	const int zmax = here->sectorZ+survivorRadius;

#   ifdef DEBUG_CACHE
		unsigned removed = 0;
#   endif
	StarSystemCache::CacheMap::const_iterator i = m_starSystemCache->Begin();
	while (i != m_starSystemCache->End()) {
		if (!WithinBox(i->second->GetPath(), xmin, xmax, ymin, ymax, zmin, zmax)) {
			m_starSystemCache->Erase(i++);
#   ifdef DEBUG_CACHE
		++removed;
#   endif
		} else
			++i;
	}
#   ifdef DEBUG_CACHE
		Output("%s: Erased %u entries.\n", StarSystemCache::CACHE_NAME.c_str(), removed);
#   endif

	SectorCache::PathVector paths;
	// build all of the possible paths we'll need to build star systems for
	for (int x = here_x-sectorRadius; x <= here_x+sectorRadius; x++) {
		for (int y = here_y-sectorRadius; y <= here_y+sectorRadius; y++) {
			for (int z = here_z-sectorRadius; z <= here_z+sectorRadius; z++) {
				SystemPath path(x, y, z);
				RefCountedPtr<Sector> sec(m_sectorCache->GetIfCached(path));
				assert(sec);
				for (const Sector::System& ss : sec->m_systems)
					paths.push_back(SystemPath(ss.sx, ss.sy, ss.sz, ss.idx));
			}
		}
	}
	m_starSystemCache->FillCache(paths);
}

void Space::GenBody(const double at_time, SystemBody *sbody, Frame *f, std::vector<vector3d> &posAccum)
{
	Body *b = 0;

	if (sbody->GetType() != SystemBody::TYPE_GRAVPOINT) {
		if (sbody->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
			Star *star = new Star(sbody);
			b = star;
		} else if ((sbody->GetType() == SystemBody::TYPE_STARPORT_ORBITAL) ||
		           (sbody->GetType() == SystemBody::TYPE_STARPORT_SURFACE)) {
			SpaceStation *ss = new SpaceStation(sbody);
			b = ss;
		} else {
			Planet *planet = new Planet(sbody);
			b = planet;
			// reset this
			posAccum.clear();
		}
		b->SetLabel(sbody->GetName().c_str());
		b->SetPosition(vector3d(0,0,0));
		AddBody(b);
	}
	f = MakeFrameFor(at_time, sbody, b, f, posAccum);

	for (SystemBody* kid : sbody->GetChildren()) {
		GenBody(at_time, kid, f, posAccum);
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
	//Output("OUCH! %x (depth %f)\n", SDL_GetTicks(), c->depth);

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
	if (altitude >= (terrain->GetMaxFeatureRadius()*2.0)) return;

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
	for (Frame* kid : f->GetChildren())
		CollideFrame(kid);
}

void Space::TimeStep(float step)
{
	PROFILE_SCOPED()

	if( Pi::MustRefreshBackgroundClearFlag() )
		RefreshBackground();

	m_frameIndexValid = m_bodyIndexValid = m_sbodyIndexValid = false;

	// XXX does not need to be done this often
	CollideFrame(m_rootFrame.get());
	for (Body* b : m_bodies)
		CollideWithTerrain(b);

	// update frames of reference
	for (Body* b : m_bodies)
		b->UpdateFrame();

	// AI acts here, then move all bodies and frames
	for (Body* b : m_bodies)
		b->StaticUpdate(step);

	m_rootFrame->UpdateOrbitRails(m_game->GetTime(), m_game->GetTimeStep());

	for (Body* b : m_bodies)
		b->TimeStepUpdate(step);

	LuaEvent::Emit();
	Pi::luaTimer->Tick();

	UpdateBodies();

	m_bodyNearFinder.Prepare();
}

void Space::UpdateBodies()
{
#ifndef NDEBUG
	m_processingFinalizationQueue = true;
#endif

	for (Body* rmb : m_removeBodies) {
		rmb->SetFrame(0);
		for (Body* b : m_bodies)
			b->NotifyRemoved(rmb);
		m_bodies.remove(rmb);
	}
	m_removeBodies.clear();

	for (Body* killb : m_killBodies) {
		for (Body* b : m_bodies)
			b->NotifyRemoved(killb);
		m_bodies.remove(killb);
		delete killb;
	}
	m_killBodies.clear();

#ifndef NDEBUG
	m_processingFinalizationQueue = false;
#endif
}

static char space[256];

static void DebugDumpFrame(Frame *f, unsigned int indent)
{
	Output("%.*s%p (%s)", indent, space, static_cast<void*>(f), f->GetLabel().c_str());
	if (f->GetParent())
		Output(" parent %p (%s)", static_cast<void*>(f->GetParent()), f->GetParent()->GetLabel().c_str());
	if (f->GetBody())
		Output(" body %p (%s)", static_cast<void*>(f->GetBody()), f->GetBody()->GetLabel().c_str());
	if (Body *b = f->GetBody())
		Output(" bodyFor %p (%s)", static_cast<void*>(b), b->GetLabel().c_str());
	Output(" distance %f radius %f", f->GetPosition().Length(), f->GetRadius());
	Output("%s\n", f->IsRotFrame() ? " [rotating]" : "");

	for (Frame* kid : f->GetChildren())
		DebugDumpFrame(kid, indent+2);
}

void Space::DebugDumpFrames()
{
	memset(space, ' ', sizeof(space));

	Output("Frame structure for '%s':\n", m_starSystem->GetName().c_str());
	DebugDumpFrame(m_rootFrame.get(), 2);
}
