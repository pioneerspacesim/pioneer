// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Space.h"

#include "Background.h"
#include "Body.h"
#include "CityOnPlanet.h"
#include "Frame.h"
#include "GameSaveError.h"
#include "HyperspaceCloud.h"
#include "Json.h"
#include "Lang.h"
#include "LuaEvent.h"
#include "LuaTimer.h"
#include "MathUtil.h"
#include "Pi.h"
#include "Planet.h"
#include "Player.h"
#include "SpaceStation.h"
#include "Star.h"
#include "collider/CollisionContact.h"
#include "collider/CollisionSpace.h"
#include "galaxy/StarSystem.h"
#include <algorithm>
#include <functional>

void Space::BodyNearFinder::Prepare()
{
	m_bodyDist.clear();

	for (Body *b : m_space->GetBodies())
		m_bodyDist.emplace_back(b, b->GetPositionRelTo(m_space->GetRootFrame()).Length());

	std::sort(m_bodyDist.begin(), m_bodyDist.end());
}

Space::BodyNearList Space::BodyNearFinder::GetBodiesMaybeNear(const Body *b, double dist)
{
	return std::move(GetBodiesMaybeNear(b->GetPositionRelTo(m_space->GetRootFrame()), dist));
}

Space::BodyNearList Space::BodyNearFinder::GetBodiesMaybeNear(const vector3d &pos, double dist)
{
	if (m_bodyDist.empty()) {
		m_nearBodies.clear();
		return std::move(m_nearBodies);
	}

	const double len = pos.Length();

	std::vector<BodyDist>::const_iterator min = std::lower_bound(m_bodyDist.begin(), m_bodyDist.end(), len - dist);
	std::vector<BodyDist>::const_iterator max = std::upper_bound(min, m_bodyDist.cend(), len + dist);

	m_nearBodies.clear();
	m_nearBodies.reserve(max - min);

	std::for_each(min, max, [&](BodyDist const &bd) { m_nearBodies.push_back(bd.body); });

	return std::move(m_nearBodies);
}

Space::Space() :
	m_frameIndexValid(false),
	m_bodyIndexValid(false),
	m_sbodyIndexValid(false),
	m_bodyNearFinder(this)
#ifndef NDEBUG
	,
	m_processingFinalizationQueue(false)
#endif
{
	m_background.reset(new Background::Container(Pi::renderer, Pi::rng));

	m_rootFrame.reset(new Frame(nullptr, Lang::SYSTEM));
	m_rootFrame->SetRadius(FLT_MAX);
}

Space::Space(double total_time, float time_step, RefCountedPtr<StarSystem> starsystem, const SystemPath &path) :
	m_starSystem(starsystem),
	m_frameIndexValid(false),
	m_bodyIndexValid(false),
	m_sbodyIndexValid(false),
	m_bodyNearFinder(this)
#ifndef NDEBUG
	,
	m_processingFinalizationQueue(false)
#endif
{
	Uint32 _init[5] = { path.systemIndex, Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rand(_init, 5);
	m_background.reset(new Background::Container(Pi::renderer, rand));

	CityOnPlanet::SetCityModelPatterns(m_starSystem->GetPath());

	// XXX set radius in constructor
	m_rootFrame.reset(new Frame(nullptr, Lang::SYSTEM));
	m_rootFrame->SetRadius(FLT_MAX);

	std::vector<vector3d> positionAccumulator;
	GenBody(total_time, m_starSystem->GetRootBody().Get(), m_rootFrame.get(), positionAccumulator);
	m_rootFrame->UpdateOrbitRails(total_time, time_step);

	//DebugDumpFrames();
}

Space::Space(RefCountedPtr<StarSystem> starsystem, const Json &jsonObj, double at_time) :
	m_starSystem(starsystem),
	m_frameIndexValid(false),
	m_bodyIndexValid(false),
	m_sbodyIndexValid(false),
	m_bodyNearFinder(this)
#ifndef NDEBUG
	,
	m_processingFinalizationQueue(false)
#endif
{
	Json spaceObj = jsonObj["space"];

	const SystemPath &path = m_starSystem->GetPath();
	Uint32 _init[5] = { path.systemIndex, Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
	Random rand(_init, 5);
	m_background.reset(new Background::Container(Pi::renderer, rand));

	RebuildSystemBodyIndex();

	CityOnPlanet::SetCityModelPatterns(m_starSystem->GetPath());

	if (!spaceObj.count("frame")) throw SavedGameCorruptException();
	m_rootFrame.reset(Frame::FromJson(spaceObj["frame"], this, 0, at_time));
	RebuildFrameIndex();

	try {
		Json bodyArray = spaceObj["bodies"].get<Json::array_t>();
		for (Uint32 i = 0; i < bodyArray.size(); i++)
			m_bodies.push_back(Body::FromJson(bodyArray[i], this));
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	RebuildBodyIndex();

	Frame::PostUnserializeFixup(m_rootFrame.get(), this);
	for (Body *b : m_bodies)
		b->PostLoadFixup(this);

	//DebugDumpFrames();
}

void Space::ToJson(Json &jsonObj)
{
	PROFILE_SCOPED()
	RebuildFrameIndex();
	RebuildBodyIndex();
	RebuildSystemBodyIndex();

	Json spaceObj({}); // Create JSON object to contain space data (all the bodies and things).

	Json frameObj({});
	Frame::ToJson(frameObj, m_rootFrame.get(), this);
	spaceObj["frame"] = frameObj;

	Json bodyArray = Json::array(); // Create JSON array to contain body data.
	for (Body *b : m_bodies) {
		Json bodyArrayEl({}); // Create JSON object to contain body.
		b->ToJson(bodyArrayEl, this);
		bodyArray.push_back(bodyArrayEl); // Append body object to array.
	}
	spaceObj["bodies"] = bodyArray; // Add body array to space object.

	jsonObj["space"] = spaceObj; // Add space object to supplied object.
}

Space::~Space()
{
	UpdateBodies(); // make sure anything waiting to be removed gets removed before we go and kill everything else
	for (std::list<Body *>::iterator i = m_bodies.begin(); i != m_bodies.end(); ++i)
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

RefCountedPtr<StarSystem> Space::GetStarSystem() const
{
	return m_starSystem;
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
	for (Frame *kid : frame->GetChildren())
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

	for (Body *b : m_bodies) {
		m_bodyIndex.push_back(b);
		// also index ships inside clouds
		// XXX we should not have to know about this. move indexing grunt work
		// down into the bodies?
		if (b->IsType(Object::HYPERSPACECLOUD)) {
			Ship *s = static_cast<HyperspaceCloud *>(b)->GetShip();
			if (s) m_bodyIndex.push_back(s);
		}
	}

	Pi::SetAmountBackgroundStars(Pi::GetAmountBackgroundStars());

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

void Space::KillBody(Body *b)
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

void Space::GetRandomOrbitFromDirection(const SystemPath &source, const SystemPath &dest,
	const vector3d &dir, vector3d &pos, vector3d &vel) const
{
	assert(m_starSystem);
	assert(source.IsSystemPath());

	assert(dest.IsSameSystem(m_starSystem->GetPath()));

	Body *primary = 0;
	if (dest.IsBodyPath()) {
		assert(dest.bodyIndex < m_starSystem->GetNumBodies());
		primary = FindBodyForPath(&dest);
		while (primary && primary->GetSystemBody()->GetSuperType() != GalaxyEnums::BodySuperType::SUPERTYPE_STAR) {
			SystemBody *parent = primary->GetSystemBody()->GetParent();
			primary = parent ? FindBodyForPath(&parent->GetPath()) : nullptr;
		}
	}
	if (!primary) {
		// find the first non-gravpoint. should be the primary star
		for (Body *b : GetBodies())
			if (b->GetSystemBody()->GetType() != GalaxyEnums::BodyType::TYPE_GRAVPOINT) {
				primary = b;
				break;
			}
	}
	assert(primary);

	// calculate distance to primary body relative to body's mass and radius
	const double max_orbit_vel = 100e3;
	double dist = G * primary->GetSystemBody()->GetMass() /
		(max_orbit_vel * max_orbit_vel);
	dist = std::max(dist, primary->GetSystemBody()->GetRadius() * 10);

	// ensure an absolut minimum distance
	dist = std::max(dist, 0.2 * AU);

	// point velocity vector along the line from source to dest,
	// make exit position perpendicular to it,
	// add random component to exit position,
	// set velocity for (almost) circular orbit
	vel = dir.Normalized();
	{
		vector3d a{ MathUtil::OrthogonalDirection(vel) };
		vector3d b{ vel.Cross(a) };
		vector3d p{ MathUtil::RandomPointOnCircle(1.) };
		pos = p.x * a + p.y * b;
	}
	pos *= dist * Pi::rng.Double(0.95, 1.2);
	vel *= sqrt(G * primary->GetSystemBody()->GetMass() / dist);

	assert(pos.Length() > primary->GetSystemBody()->GetRadius());
	pos += primary->GetPositionRelTo(GetRootFrame());
}

Body *Space::FindNearestTo(const Body *b, Object::Type t) const
{
	Body *nearest = 0;
	double dist = FLT_MAX;
	for (std::list<Body *>::const_iterator i = m_bodies.begin(); i != m_bodies.end(); ++i) {
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

	for (Body *b : m_bodies) {
		if (b->GetSystemBody() == body) return b;
	}
	return 0;
}

static Frame *find_frame_with_sbody(Frame *f, const SystemBody *b)
{
	if (f->GetSystemBody() == b)
		return f;
	else {
		for (Frame *kid : f->GetChildren()) {
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
	pos = rot * vector3d(0, 1, 0);

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
	const double delta = 20.0 / radius; // in radii
	const double maxSlope = 0.2; // 0.0 to 1.0
	const double maxHeightVariation = maxSlope * delta * radius; // in m

	matrix3x3d rot_ = rot;
	vector3d pos_ = pos;

	const bool manualRelocationIsEasy = !(planet->GetSystemBody()->GetType() == GalaxyEnums::BodyType::TYPE_PLANET_ASTEROID || terrainHeightVariation > heightVariationCheckThreshold);

	// warn and leave it up to the user to relocate custom starports when it's easy to relocate manually, i.e. not on asteroids and other planets which are likely to have high variation in a lot of places
	const bool isRelocatableIfBuried = !(sbody->IsCustomBody() && manualRelocationIsEasy);

	bool isInitiallyUnderwater = false;
	bool initialVariationTooHigh = false;

	Random r(sbody->GetSeed());

	for (int tries = 0; tries < 200; tries++) {
		variationWithinLimits = true;

		const double height = planet->GetTerrainHeight(pos_) - radius; // in m

		// check height at 6 points around the starport center stays within variation tolerances
		// GetHeight gives a varying height field in 3 dimensions.
		// Given it's smoothly varying it's fine to sample it in arbitary directions to get an idea of how sharply it varies
		double v[6];
		v[0] = fabs(planet->GetTerrainHeight(vector3d(pos_.x + delta, pos_.y, pos_.z)) - radius - height);
		v[1] = fabs(planet->GetTerrainHeight(vector3d(pos_.x - delta, pos_.y, pos_.z)) - radius - height);
		v[2] = fabs(planet->GetTerrainHeight(vector3d(pos_.x, pos_.y, pos_.z + delta)) - radius - height);
		v[3] = fabs(planet->GetTerrainHeight(vector3d(pos_.x, pos_.y, pos_.z - delta)) - radius - height);
		v[4] = fabs(planet->GetTerrainHeight(vector3d(pos_.x, pos_.y + delta, pos_.z)) - radius - height);
		v[5] = fabs(planet->GetTerrainHeight(vector3d(pos_.x, pos_.y - delta, pos_.z)) - radius - height);

		// break if variation for all points is within limits
		double variationMax = 0.0;
		for (int i = 0; i < 6; i++) {
			variationWithinLimits = variationWithinLimits && (v[i] < maxHeightVariation);
			variationMax = (v[i] > variationMax) ? v[i] : variationMax;
		}

		// check if underwater
		const bool starportUnderwater = (height <= 0.0);

		//Output("%s: try no: %i, Match found: %i, best variation in previous results %f, variationMax this try: %f, maxHeightVariation: %f, Starport is underwater: %i\n",
		//	sbody->name.c_str(), tries, (variationWithinLimits && !starportUnderwater), bestVariation, variationMax, maxHeightVariation, starportUnderwater);

		bool tooCloseToOther = false;
		for (vector3d oldPos : prevPositions) {
			// is the distance between points less than the delta distance?
			if ((pos_ - oldPos).LengthSqr() < (delta * delta)) {
				tooCloseToOther = true; // then we're too close so try again
				break;
			}
		}

		if (tries == 0) {
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
		const double r2 = r.Double(); // function parameter evaluation order is implementation-dependent
		const double r1 = r.Double(); // can't put two rands in the same expression
		rot_ = matrix3x3d::RotateZ(2.0 * M_PI * r1) * matrix3x3d::RotateY(2.0 * M_PI * r2) * matrix3x3d::RotateX(2.0 * M_PI * r3);
		pos_ = rot_ * vector3d(0, 1, 0);
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

	if (sbody->GetType() == GalaxyEnums::BodyType::TYPE_GRAVPOINT) {
		Frame *orbFrame = new Frame(f, sbody->GetName().c_str());
		orbFrame->SetBodies(sbody, b);
		orbFrame->SetRadius(sbody->GetMaxChildOrbitalDistance() * 1.1);
		return orbFrame;
	}

	GalaxyEnums::BodySuperType supertype = sbody->GetSuperType();

	if ((supertype == GalaxyEnums::BodySuperType::SUPERTYPE_GAS_GIANT) ||
		(supertype == GalaxyEnums::BodySuperType::SUPERTYPE_ROCKY_PLANET)) {
		// for planets we want an non-rotating frame for a few radii
		// and a rotating frame with no radius to contain attached objects
		double frameRadius = std::max(4.0 * sbody->GetRadius(), sbody->GetMaxChildOrbitalDistance() * 1.05);
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
		double angSpeed = 2.0 * M_PI / sbody->GetRotationPeriod();
		rotFrame->SetAngSpeed(angSpeed);

		if (sbody->HasRotationPhase())
			rotMatrix = rotMatrix * matrix3x3d::RotateY(sbody->GetRotationPhaseAtStart());
		rotFrame->SetInitialOrient(rotMatrix, at_time);

		b->SetFrame(rotFrame);
		return orbFrame;
	} else if (supertype == GalaxyEnums::BodySuperType::SUPERTYPE_STAR) {
		// stars want a single small non-rotating frame
		// bigger than it's furtherest orbiting body.
		// if there are no orbiting bodies use a frame of several radii.
		Frame *orbFrame = new Frame(f, sbody->GetName().c_str());
		orbFrame->SetBodies(sbody, b);
		const double bodyRadius = sbody->GetEquatorialRadius();
		double frameRadius = std::max(10.0 * bodyRadius, sbody->GetMaxChildOrbitalDistance() * 1.1);
		// Respect the frame of other stars in the multi-star system. We still make sure that the frame ends outside
		// the body. For a minimum separation of 1.236 radii, nothing will overlap (see StarSystem::StarSystem()).
		if (sbody->GetParent() && frameRadius > AU * 0.11 * sbody->GetOrbMin())
			frameRadius = std::max(1.1 * bodyRadius, AU * 0.11 * sbody->GetOrbMin());
		orbFrame->SetRadius(frameRadius);
		b->SetFrame(orbFrame);
		return orbFrame;
	} else if (sbody->GetType() == GalaxyEnums::BodyType::TYPE_STARPORT_ORBITAL) {
		// space stations want non-rotating frame to some distance
		Frame *orbFrame = new Frame(f, sbody->GetName().c_str());
		orbFrame->SetBodies(sbody, b);
		//		orbFrame->SetRadius(10*sbody->GetRadius());
		orbFrame->SetRadius(20000.0); // 4x standard parking radius
		b->SetFrame(orbFrame);
		return orbFrame;

	} else if (sbody->GetType() == GalaxyEnums::BodyType::TYPE_STARPORT_SURFACE) {
		// just put body into rotating frame of planet, not in its own frame
		// (because collisions only happen between objects in same frame,
		// and we want collisions on starport and on planet itself)
		Frame *rotFrame = f->GetRotFrame();
		b->SetFrame(rotFrame);
		assert(rotFrame->IsRotFrame());
		assert(rotFrame->GetBody()->IsType(Object::PLANET));
		matrix3x3d rot;
		vector3d pos;
		Planet *planet = static_cast<Planet *>(rotFrame->GetBody());
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

void Space::GenBody(const double at_time, SystemBody *sbody, Frame *f, std::vector<vector3d> &posAccum)
{
	Body *b = nullptr;

	if (sbody->GetType() != GalaxyEnums::BodyType::TYPE_GRAVPOINT) {
		if (sbody->GetSuperType() == GalaxyEnums::BodySuperType::SUPERTYPE_STAR) {
			Star *star = new Star(sbody);
			b = star;
		} else if ((sbody->GetType() == GalaxyEnums::BodyType::TYPE_STARPORT_ORBITAL) ||
			(sbody->GetType() == GalaxyEnums::BodyType::TYPE_STARPORT_SURFACE)) {
			SpaceStation *ss = new SpaceStation(sbody);
			b = ss;
		} else {
			Planet *planet = new Planet(sbody);
			b = planet;
			// reset this
			posAccum.clear();
		}
		b->SetLabel(sbody->GetName().c_str());
		b->SetPosition(vector3d(0, 0, 0));
		AddBody(b);
	}
	f = MakeFrameFor(at_time, sbody, b, f, posAccum);

	for (SystemBody *kid : sbody->GetChildren()) {
		GenBody(at_time, kid, f, posAccum);
	}
}

static bool OnCollision(Object *o1, Object *o2, CollisionContact *c, double relativeVel)
{
	Body *pb1 = static_cast<Body *>(o1);
	Body *pb2 = static_cast<Body *>(o2);
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

	Object *po1 = static_cast<Object *>(c->userData1);
	Object *po2 = static_cast<Object *>(c->userData2);

	const bool po1_isDynBody = po1->IsType(Object::DYNAMICBODY);
	const bool po2_isDynBody = po2->IsType(Object::DYNAMICBODY);
	// collision response
	assert(po1_isDynBody || po2_isDynBody);

	// Bounce factor
	const double coeff_rest = 0.35;
	// Allow stop due to friction
	const double coeff_slide = 0.700;

	if (po1_isDynBody && po2_isDynBody) {
		DynamicBody *b1 = static_cast<DynamicBody *>(po1);
		DynamicBody *b2 = static_cast<DynamicBody *>(po2);
		const vector3d linVel1 = b1->GetVelocity();
		const vector3d linVel2 = b2->GetVelocity();
		const vector3d angVel1 = b1->GetAngVelocity();
		const vector3d angVel2 = b2->GetAngVelocity();

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
		const double term3 = c->normal.Dot((hitPos1.Cross(c->normal) * invAngInert1).Cross(hitPos1));
		const double term4 = c->normal.Dot((hitPos2.Cross(c->normal) * invAngInert2).Cross(hitPos2));

		const double j = numerator / (term1 + term2 + term3 + term4);
		const vector3d force = j * c->normal;

		b1->SetVelocity(linVel1 * (1 - coeff_slide * c->timestep) + force * invMass1);
		b1->SetAngVelocity(angVel1 + hitPos1.Cross(force) * invAngInert1);
		b2->SetVelocity(linVel2 * (1 - coeff_slide * c->timestep) - force * invMass2);
		b2->SetAngVelocity(angVel2 - hitPos2.Cross(force) * invAngInert2);
	} else {
		// one body is static
		vector3d hitNormal;
		DynamicBody *mover;

		if (po1_isDynBody) {
			mover = static_cast<DynamicBody *>(po1);
			hitNormal = c->normal;
		} else {
			mover = static_cast<DynamicBody *>(po2);
			hitNormal = -c->normal;
		}

		const vector3d linVel1 = mover->GetVelocity();
		const vector3d angVel1 = mover->GetAngVelocity();

		// step back
		//		mover->UndoTimestep();

		const double invMass1 = 1.0 / mover->GetMass();
		const vector3d hitPos1 = c->pos - mover->GetPosition();
		const vector3d hitVel1 = linVel1 + angVel1.Cross(hitPos1);
		const double relVel = hitVel1.Dot(c->normal);
		// moving away so no collision
		if (relVel > 0) return;
		if (!OnCollision(po1, po2, c, -relVel)) return;
		const double invAngInert = 1.0 / mover->GetAngularInertia();
		const double numerator = -(1.0 + coeff_rest) * relVel;
		const double term1 = invMass1;
		const double term3 = c->normal.Dot((hitPos1.Cross(c->normal) * invAngInert).Cross(hitPos1));

		const double j = numerator / (term1 + term3);
		const vector3d force = j * c->normal;

		/*
		   "Linear projection reduces the penetration of two
		   objects by a small percentage, and this is performed
		   after the impulse is applied"

		   From:
		   https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331

		   Correction should never be more than c->depth (std::min) and never negative (std::max)
		   NOTE: usually instead of 'c->timestep' you find a 'percent',
		   but here we have a variable timestep and thus the upper limit,
		   which is intended to trigger a collision in the subsequent frame.
		   NOTE2: works (as intendend) at low timestep.
		   Further improvement could be:
				1) velocity should be projected relative to normal direction,
				   so bouncing and friction may act on the "correct" components
				   (though that a reason for fails and glitches are frames skipped
				   because relVel is quitting before any further calculation)
				2) with time accel at 10000x you end in space... probably in order
				   for that to works correctly some deeper change should kick in
		*/
		const float threshold = 0.005;

		vector3d correction = std::min(std::max(c->depth - threshold, 0.0) * c->timestep, c->depth + threshold) * c->normal;

		mover->SetPosition(mover->GetPosition() + correction);

		const float reduction = std::max(1 - coeff_slide * c->timestep, 0.0);
		vector3d final_vel = linVel1 * reduction + force * invMass1;
		if (final_vel.LengthSqr() < 0.1) final_vel = vector3d(0.0);

		mover->SetVelocity(final_vel);
		mover->SetAngVelocity(angVel1 + hitPos1.Cross(force) * invAngInert);
	}
}

// temporary one-point version
static void CollideWithTerrain(Body *body, float timeStep)
{
	if (!body->IsType(Object::DYNAMICBODY))
		return;
	DynamicBody *dynBody = static_cast<DynamicBody *>(body);
	if (!dynBody->IsMoving())
		return;

	Frame *f = body->GetFrame();
	if (!f || !f->GetBody() || f != f->GetBody()->GetFrame())
		return;
	if (!f->GetBody()->IsType(Object::TERRAINBODY))
		return;
	TerrainBody *terrain = static_cast<TerrainBody *>(f->GetBody());

	const Aabb &aabb = dynBody->GetAabb();
	double altitude = body->GetPosition().Length() + aabb.min.y;
	if (altitude >= (terrain->GetMaxFeatureRadius() * 2.0))
		return;

	double terrHeight = terrain->GetTerrainHeight(body->GetPosition().Normalized());
	if (altitude >= terrHeight)
		return;

	CollisionContact c(body->GetPosition(), body->GetPosition().Normalized(), terrHeight - altitude, timeStep, static_cast<void *>(body), static_cast<void *>(f->GetBody()));
	hitCallback(&c);
}

void Space::CollideFrame(Frame *f)
{
	f->GetCollisionSpace()->Collide(&hitCallback);
	for (Frame *kid : f->GetChildren())
		CollideFrame(kid);
}

void Space::TimeStep(float step, double total_time)
{
	PROFILE_SCOPED()

	if (Pi::MustRefreshBackgroundClearFlag())
		RefreshBackground();

	m_frameIndexValid = m_bodyIndexValid = m_sbodyIndexValid = false;

	// XXX does not need to be done this often
	CollideFrame(m_rootFrame.get());
	for (Body *b : m_bodies)
		CollideWithTerrain(b, step);

	// update frames of reference
	for (Body *b : m_bodies)
		b->UpdateFrame();

	// AI acts here, then move all bodies and frames
	for (Body *b : m_bodies)
		b->StaticUpdate(step);

	m_rootFrame->UpdateOrbitRails(total_time, step);

	for (Body *b : m_bodies)
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

	for (Body *rmb : m_removeBodies) {
		rmb->SetFrame(0);
		for (Body *b : m_bodies)
			b->NotifyRemoved(rmb);
		m_bodies.remove(rmb);
	}
	m_removeBodies.clear();

	for (Body *killb : m_killBodies) {
		for (Body *b : m_bodies)
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
	Output("%.*s%p (%s)", indent, space, static_cast<void *>(f), f->GetLabel().c_str());
	if (f->GetParent())
		Output(" parent %p (%s)", static_cast<void *>(f->GetParent()), f->GetParent()->GetLabel().c_str());
	if (f->GetBody())
		Output(" body %p (%s)", static_cast<void *>(f->GetBody()), f->GetBody()->GetLabel().c_str());
	if (Body *b = f->GetBody())
		Output(" bodyFor %p (%s)", static_cast<void *>(b), b->GetLabel().c_str());
	Output(" distance %f radius %f", f->GetPosition().Length(), f->GetRadius());
	Output("%s\n", f->IsRotFrame() ? " [rotating]" : "");

	for (Frame *kid : f->GetChildren())
		DebugDumpFrame(kid, indent + 2);
}

void Space::DebugDumpFrames()
{
	memset(space, ' ', sizeof(space));

	Output("Frame structure for '%s':\n", m_starSystem->GetName().c_str());
	DebugDumpFrame(m_rootFrame.get(), 2);
}
