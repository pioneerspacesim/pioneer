// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Space.h"

#include "Body.h"
#include "CityOnPlanet.h"
#include "Frame.h"
#include "Game.h"
#include "GameSaveError.h"
#include "HyperspaceCloud.h"
#include "JsonUtils.h"
#include "Lang.h"
#include "MathUtil.h"
#include "Pi.h"
#include "Planet.h"
#include "Player.h"
#include "SpaceStation.h"
#include "Star.h"
#include "SystemView.h"
#include "collider/CollisionContact.h"
#include "collider/CollisionSpace.h"
#include "core/Log.h"
#include "galaxy/Galaxy.h"
#include "graphics/Graphics.h"
#include "lua/LuaEvent.h"
#include "lua/LuaTimer.h"
#include <algorithm>
#include <functional>

//#define DEBUG_CACHE

static void RelocateStarportIfNecessary(SystemBody *sbody, Planet *planet, vector3d &pos, matrix3x3d &rot, const std::vector<vector3d> &prevPositions)
{
	const double radius = planet->GetSystemBody()->GetRadius();

	// suggested position
	rot = sbody->GetOrbit().GetPlane();
	pos = rot * vector3d(0, 1, 0);

	// Check if height varies too much around the starport center
	// by sampling 6 points around it. Try up to 100 new positions randomly until a match is found.
	// This is not guaranteed to find a match but greatly increases the chancessteroids which are not too steep.

	bool variationWithinLimits = true;
	double bestVariation = 1e10; // any high value
	matrix3x3d rotNotUnderwaterWithLeastVariation = rot;
	vector3d posNotUnderwaterWithLeastVariation = pos;
	const double heightVariationCheckThreshold = 0.008;					 // max variation to radius radius ratio to check for local slope, ganymede is around 0.01
	const double terrainHeightVariation = planet->GetMaxFeatureRadius(); //in radii

	//Output("%s: terrain height variation %f\n", sbody->name.c_str(), terrainHeightVariation);

	// 6 points are sampled around the starport center by adding/subtracting delta to to coords
	// points must stay within max height variation to be accepted
	//    1. delta should be chosen such that it a distance from the starport center that encloses landing pads for the largest starport
	//    2. maxSlope should be set so maxHeightVariation is less than the height of the landing pads
	const double delta = 20.0 / radius;							 // in radii
	const double maxSlope = 0.2;								 // 0.0 to 1.0
	const double maxHeightVariation = maxSlope * delta * radius; // in m

	matrix3x3d rot_ = rot;
	vector3d pos_ = pos;

	const bool manualRelocationIsEasy = !(planet->GetSystemBody()->GetType() == SystemBody::TYPE_PLANET_ASTEROID || terrainHeightVariation > heightVariationCheckThreshold);

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
		// Given it's smoothly varying it's fine to sample it in arbitrary directions to get an idea of how sharply it varies
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

void Space::BodyNearFinder::Prepare()
{
	PROFILE_SCOPED()
	m_bodyDist.clear();

	for (Body *b : m_space->GetBodies())
		m_bodyDist.emplace_back(b, b->GetPositionRelTo(m_space->GetRootFrame()).Length());

	std::sort(m_bodyDist.begin(), m_bodyDist.end());
}

Space::BodyNearList Space::BodyNearFinder::GetBodiesMaybeNear(const Body *b, double dist)
{
	return GetBodiesMaybeNear(b->GetPositionRelTo(m_space->GetRootFrame()), dist);
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

Space::Space(Game *game, RefCountedPtr<Galaxy> galaxy, Space *oldSpace) :
	m_starSystemCache(oldSpace ? oldSpace->m_starSystemCache : galaxy->NewStarSystemSlaveCache()),
	m_game(game),
	m_bodyIndexValid(false),
	m_sbodyIndexValid(false),
	m_bodyNearFinder(this)
#ifndef NDEBUG
	,
	m_processingFinalizationQueue(false)
#endif
{
	RefreshBackground();

	m_rootFrameId = Frame::CreateFrame(FrameId::Invalid, Lang::SYSTEM, Frame::FLAG_DEFAULT, FLT_MAX);

	GenSectorCache(galaxy, &game->GetHyperspaceDest());
}

Space::Space(Game *game, RefCountedPtr<Galaxy> galaxy, const SystemPath &path, Space *oldSpace) :
	m_starSystemCache(oldSpace ? oldSpace->m_starSystemCache : galaxy->NewStarSystemSlaveCache()),
	m_starSystem(galaxy->GetStarSystem(path)),
	m_game(game),
	m_bodyIndexValid(false),
	m_sbodyIndexValid(false),
	m_bodyNearFinder(this)
#ifndef NDEBUG
	,
	m_processingFinalizationQueue(false)
#endif
{
	PROFILE_SCOPED()
	RefreshBackground();

	CityOnPlanet::SetCityModelPatterns(m_starSystem->GetPath());

	m_rootFrameId = Frame::CreateFrame(FrameId::Invalid, Lang::SYSTEM, Frame::FLAG_DEFAULT, FLT_MAX);

	std::vector<vector3d> positionAccumulator;
	GenBody(m_game->GetTime(), m_starSystem->GetRootBody().Get(), m_rootFrameId, positionAccumulator);
	Frame::UpdateOrbitRails(m_game->GetTime(), m_game->GetTimeStep());

	GenSectorCache(galaxy, &path);
}

Space::Space(Game *game, RefCountedPtr<Galaxy> galaxy, const Json &jsonObj, double at_time) :
	m_starSystemCache(galaxy->NewStarSystemSlaveCache()),
	m_game(game),
	m_bodyIndexValid(false),
	m_sbodyIndexValid(false),
	m_bodyNearFinder(this)
#ifndef NDEBUG
	,
	m_processingFinalizationQueue(false)
#endif
{
	PROFILE_SCOPED()
	Json spaceObj = jsonObj["space"];

	m_starSystem = StarSystem::FromJson(galaxy, spaceObj);

	RefreshBackground();

	RebuildSystemBodyIndex();

	CityOnPlanet::SetCityModelPatterns(m_starSystem->GetPath());

	if (!spaceObj.count("frame")) throw SavedGameCorruptException();
	m_rootFrameId = Frame::FromJson(spaceObj["frame"], this, FrameId::Invalid, at_time);

	try {
		Json bodyArray = spaceObj["bodies"].get<Json::array_t>();
		for (Uint32 i = 0; i < bodyArray.size(); i++) {
			if (bodyArray[i].count("is_not_in_space") > 0)
				continue;
			m_bodies.push_back(Body::FromJson(bodyArray[i], this));
		}
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	RebuildBodyIndex();

	Frame::PostUnserializeFixup(m_rootFrameId, this);
	for (Body *b : m_bodies)
		b->PostLoadFixup(this);

	// some spaceports could be moved, now their physical bodies were loaded from
	// json, with offsets, now we should move the system bodies also so that
	// there is no mismatch
	for (auto sbody : m_starSystem->GetBodies()) {
		if (sbody->GetSuperType() == SystemBody::SUPERTYPE_ROCKY_PLANET) {
			// needs a clean posAccum for each planet
			std::vector<vector3d> posAccum;
			SystemPath planet_path = m_starSystem->GetPathOf(sbody.Get());
			Planet *planet = static_cast<Planet *>(FindBodyForPath(&planet_path));
			for (auto kid : sbody->GetChildren()) {
				if (kid->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
					// out arguments
					matrix3x3d rot;
					vector3d pos;
					RelocateStarportIfNecessary(kid, planet, pos, rot, posAccum);
					// the surface starport's location is stored in its "orbit", as orientation matrix
					kid->SetOrbitPlane(rot);
					// accumulate for testing against
					posAccum.push_back(pos);
				}
			}
		}
	}

	GenSectorCache(galaxy, &m_starSystem->GetPath());

	//DebugDumpFrames();
}

Space::~Space()
{
	UpdateBodies(); // make sure anything waiting to be removed gets removed before we go and kill everything else
	for (Body *body : m_bodies)
		KillBody(body);
	UpdateBodies();

	// since the player is owned by the game, we cannot delete it, but it
	// stores the id of the frame we are going to delete
	auto player = m_game->GetPlayer();
	if (player) player->SetFrame(FrameId::Invalid);

	Frame::DeleteFrames();
}

void Space::RefreshBackground()
{
	PROFILE_SCOPED()
	if (m_starSystem.Valid()) {
		const SystemPath &path = m_starSystem->GetPath();
		Uint32 _init[5] = { path.systemIndex, Uint32(path.sectorX), Uint32(path.sectorY), Uint32(path.sectorZ), UNIVERSE_SEED };
		Random rand(_init, 5);
		m_background.reset(new Background::Container(Pi::renderer, rand));
		m_background->GetStarfield()->Fill(rand, &this->GetStarSystem()->GetPath(), m_game->GetGalaxy());
	} else {
		m_background.reset(new Background::Container(Pi::renderer, Pi::rng));
		m_background->GetStarfield()->Fill(Pi::rng, nullptr, m_game->GetGalaxy());
	}
}

void Space::ToJson(Json &jsonObj)
{
	PROFILE_SCOPED()
	RebuildBodyIndex();
	RebuildSystemBodyIndex();

	Json spaceObj({}); // Create JSON object to contain space data (all the bodies and things).

	StarSystem::ToJson(spaceObj, m_starSystem.Get());

	Json frameObj({});
	Frame::ToJson(frameObj, m_rootFrameId, this);
	spaceObj["frame"] = frameObj;

	Json bodyArray = Json::array(); // Create JSON array to contain body data.
	for (size_t i = 0; i < m_bodyIndex.size() - 1; i++) {
		// First index of m_bodyIndex is reserved to
		// nullptr or bad index
		Body *b = m_bodyIndex[i + 1];
		Json bodyArrayEl({}); // Create JSON object to contain body.
		if (!b->IsInSpace()) {
			bodyArrayEl["is_not_in_space"] = true;
			// Append empty body object to array.
			// The only working example right now is ship in hyperspace
			// which is loaded through HyperspaceCloud class, so
			// there is no need to load it a second time.
			// FIXME: This is done to save it's lua components which is otherwise won't save.
			// This needs a better way to handle this case.
			// Described in depth: https://github.com/pioneerspacesim/pioneer/pull/5657#issuecomment-1818188703
			bodyArray.push_back(bodyArrayEl);
			continue;
		}
		b->ToJson(bodyArrayEl, this);
		bodyArray.push_back(bodyArrayEl); // Append body object to array.
	}
	spaceObj["bodies"] = bodyArray; // Add body array to space object.

	jsonObj["space"] = spaceObj; // Add space object to supplied object.
}

Body *Space::GetBodyByIndex(Uint32 idx) const
{
	assert(m_bodyIndexValid);
	assert(m_bodyIndex.size() > idx);
	if (idx == SDL_MAX_UINT32 || m_bodyIndex.size() <= idx) {
		Output("GetBodyByIndex passed bad index %u", idx);
		return nullptr;
	}
	return m_bodyIndex[idx];
}

SystemBody *Space::GetSystemBodyByIndex(Uint32 idx) const
{
	assert(m_sbodyIndexValid);
	assert(m_sbodyIndex.size() > idx);
	return m_sbodyIndex[idx];
}

Uint32 Space::GetIndexForBody(const Body *body) const
{
	assert(m_bodyIndexValid);
	for (Uint32 i = 0; i < m_bodyIndex.size(); i++)
		if (m_bodyIndex[i] == body) return i;
	assert(false);
	Output("GetIndexForBody passed unknown body");
	return SDL_MAX_UINT32;
}

Uint32 Space::GetIndexForSystemBody(const SystemBody *sbody) const
{
	assert(m_sbodyIndexValid);
	for (Uint32 i = 0; i < m_sbodyIndex.size(); i++)
		if (m_sbodyIndex[i] == sbody) return i;
	assert(0);
	return SDL_MAX_UINT32;
}

void Space::AddSystemBodyToIndex(SystemBody *sbody)
{
	assert(sbody);
	m_sbodyIndex.push_back(sbody);
	for (Uint32 i = 0; i < sbody->GetNumChildren(); i++)
		AddSystemBodyToIndex(sbody->GetChildren()[i]);
}

void Space::RebuildBodyIndex()
{
	m_bodyIndex.clear();
	m_bodyIndex.push_back(nullptr);

	for (Body *b : m_bodies) {
		m_bodyIndex.push_back(b);
		// also index ships inside clouds
		// XXX we should not have to know about this. move indexing grunt work
		// down into the bodies?
		if (b->IsType(ObjectType::HYPERSPACECLOUD)) {
			Ship *s = static_cast<HyperspaceCloud *>(b)->GetShip();
			if (s) m_bodyIndex.push_back(s);
		}
	}

	Pi::SetAmountBackgroundStars(Pi::GetAmountBackgroundStars());
	Pi::SetStarFieldStarSizeFactor(Pi::GetStarFieldStarSizeFactor());

	m_bodyIndexValid = true;
}

void Space::RebuildSystemBodyIndex()
{
	m_sbodyIndex.clear();
	m_sbodyIndex.push_back(nullptr);

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
	m_assignedBodies.emplace_back(b, BodyAssignation::REMOVE);
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
			m_assignedBodies.emplace_back(b, BodyAssignation::KILL);
	}
}

void Space::GetHyperspaceExitParams(const SystemPath &source, const SystemPath &dest,
	vector3d &pos, vector3d &vel) const
{
	assert(m_starSystem);
	assert(source.IsSystemPath());

	assert(dest.IsSameSystem(m_starSystem->GetPath()));

	RefCountedPtr<const Sector> source_sec = m_sectorCache->GetCached(source);
	RefCountedPtr<const Sector> dest_sec = m_sectorCache->GetCached(dest);

	Sector::System source_sys = source_sec->m_systems[source.systemIndex];
	Sector::System dest_sys = dest_sec->m_systems[dest.systemIndex];

	const vector3d sourcePos = vector3d(source_sys.GetFullPosition());
	const vector3d destPos = vector3d(dest_sys.GetFullPosition());

	Body *primary = 0;
	if (dest.IsBodyPath()) {
		assert(dest.bodyIndex < m_starSystem->GetNumBodies());
		primary = FindBodyForPath(&dest);
		while (primary && primary->GetSystemBody()->GetSuperType() != SystemBody::SUPERTYPE_STAR) {
			SystemBody *parent = primary->GetSystemBody()->GetParent();
			primary = parent ? FindBodyForPath(&parent->GetPath()) : 0;
		}
	}
	if (!primary) {
		// find the first non-gravpoint. should be the primary star
		for (Body *b : GetBodies())
			if (b->GetSystemBody()->GetType() != SystemBody::TYPE_GRAVPOINT) {
				primary = b;
				break;
			}
	}
	assert(primary);

	// calculate distance to primary body relative to body's mass and radius
	const double max_orbit_vel = 100e3;
	double dist = G * primary->GetSystemBody()->GetMass() /
		(max_orbit_vel * max_orbit_vel);

	// ensure an absolute minimum and an absolute maximum distance
	// the minimum distance from the center of the star should not be less than the radius of the star
	dist = Clamp(dist, primary->GetSystemBody()->GetRadius() * 1.1, std::max(primary->GetSystemBody()->GetRadius() * 1.1, 100 * AU));

	// point velocity vector along the line from source to dest,
	// make exit position perpendicular to it,
	// add random component to exit position,
	// set velocity for (almost) circular orbit
	vel = (destPos - sourcePos).Normalized();
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

Body *Space::FindNearestTo(const Body *b, ObjectType t) const
{
	Body *nearest = 0;
	double dist = FLT_MAX;
	for (Body *const body : m_bodies) {
		if (body->IsDead()) continue;
		if (body->IsType(t)) {
			double d = body->GetPositionRelTo(b).Length();
			if (d < dist) {
				dist = d;
				nearest = body;
			}
		}
	}
	return nearest;
}

std::vector<Space::BodyDist> Space::BodiesInAngle(const Body *b, const vector3d &offset, const vector3d &view_dir, double cosOfMaxAngle) const
{
	std::vector<BodyDist> ret;
	for (Body *const body : m_bodies) {
		if (body == b) continue;
		if (body->IsDead()) continue;

		//offset from the body center - like for view from ship cocpit
		vector3d dirBody = body->GetPositionRelTo(b) * b->GetOrient() - offset;
		double d = dirBody.Length();
		//Normalizing but not using Normalized() function to avoid calculating Length again
		dirBody = dirBody / d;

		//Bodies outside of the cone disregarded
		if (dirBody.Dot(view_dir) < cosOfMaxAngle)
			continue;

		ret.emplace_back(body, d);
	}
	return ret;
}

Body *Space::FindBodyForPath(const SystemPath *path) const
{
	if (!m_game->IsNormalSpace() || !path->IsSameSystem(m_starSystem->GetPath()))
		return nullptr;

	// it is a bit dumb that currentSystem is not part of Space...
	SystemBody *body = m_starSystem->GetBodyByPath(path);
	if (!body)
		return nullptr;

	for (Body *b : m_bodies) {
		if (b->GetSystemBody() == body) return b;
	}

	return nullptr;
}

static FrameId find_frame_with_sbody(FrameId fId, const SystemBody *b)
{
	Frame *f = Frame::GetFrame(fId);
	if (f->GetSystemBody() == b)
		return fId;
	else {
		for (FrameId kid : f->GetChildren()) {
			FrameId found = find_frame_with_sbody(kid, b);
			if (found.valid())
				return found;
		}
	}
	return FrameId::Invalid;
}

FrameId Space::GetFrameWithSystemBody(const SystemBody *b) const
{
	return find_frame_with_sbody(m_rootFrameId, b);
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
	SectorDistanceSort(const SystemPath *centre) :
		here(centre)
	{
	}

private:
	SectorDistanceSort() {}
	SystemPath here;
};

void Space::GenSectorCache(RefCountedPtr<Galaxy> galaxy, const SystemPath *here)
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
	for (int x = here_x - sectorRadius; x <= here_x + sectorRadius; x++) {
		for (int y = here_y - sectorRadius; y <= here_y + sectorRadius; y++) {
			for (int z = here_z - sectorRadius; z <= here_z + sectorRadius; z++) {
				SystemPath path(x, y, z);
				paths.push_back(path);
			}
		}
	}
	// sort them so that those closest to the "here" path are processed first
	SectorDistanceSort SDS(here);
	std::sort(paths.begin(), paths.end(), SDS);
	m_sectorCache = galaxy->NewSectorSlaveCache();
	const SystemPath &center(*here);
	m_sectorCache->FillCache(paths, [this, center]() { UpdateStarSystemCache(&center); });
}

static bool WithinBox(const SystemPath &here, const int Xmin, const int Xmax, const int Ymin, const int Ymax, const int Zmin, const int Zmax)
{
	PROFILE_SCOPED()
	if (here.sectorX >= Xmin && here.sectorX <= Xmax) {
		if (here.sectorY >= Ymin && here.sectorY <= Ymax) {
			if (here.sectorZ >= Zmin && here.sectorZ <= Zmax) {
				return true;
			}
		}
	}
	return false;
}

void Space::UpdateStarSystemCache(const SystemPath *here)
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
	static const int survivorRadius = sectorRadius * 3;

	// min/max box limits
	const int xmin = here->sectorX - survivorRadius;
	const int xmax = here->sectorX + survivorRadius;
	const int ymin = here->sectorY - survivorRadius;
	const int ymax = here->sectorY + survivorRadius;
	const int zmin = here->sectorZ - survivorRadius;
	const int zmax = here->sectorZ + survivorRadius;

#ifdef DEBUG_CACHE
	unsigned removed = 0;
#endif
	StarSystemCache::CacheMap::const_iterator i = m_starSystemCache->Begin();
	while (i != m_starSystemCache->End()) {
		if (!WithinBox(i->second->GetPath(), xmin, xmax, ymin, ymax, zmin, zmax)) {
			m_starSystemCache->Erase(i++);
#ifdef DEBUG_CACHE
			++removed;
#endif
		} else
			++i;
	}
#ifdef DEBUG_CACHE
	Output("%s: Erased %u entries.\n", StarSystemCache::CACHE_NAME.c_str(), removed);
#endif

	SectorCache::PathVector paths;
	// build all of the possible paths we'll need to build star systems for
	for (int x = here_x - sectorRadius; x <= here_x + sectorRadius; x++) {
		for (int y = here_y - sectorRadius; y <= here_y + sectorRadius; y++) {
			for (int z = here_z - sectorRadius; z <= here_z + sectorRadius; z++) {
				SystemPath path(x, y, z);
				RefCountedPtr<Sector> sec(m_sectorCache->GetIfCached(path));
				assert(sec);
				for (const Sector::System &ss : sec->m_systems)
					paths.push_back(SystemPath(ss.sx, ss.sy, ss.sz, ss.idx));
			}
		}
	}
	m_starSystemCache->FillCache(paths);
}

static FrameId MakeFramesFor(const double at_time, SystemBody *sbody, Body *b, FrameId fId, std::vector<vector3d> &prevPositions)
{
	PROFILE_SCOPED()
	if (!sbody->GetParent()) {
		if (b) b->SetFrame(fId);
		Frame *f = Frame::GetFrame(fId);
		f->SetBodies(sbody, b);
		return fId;
	}

	if (sbody->GetType() == SystemBody::TYPE_GRAVPOINT) {
		FrameId orbFrameId = Frame::CreateFrame(fId,
			sbody->GetName().c_str(),
			Frame::FLAG_DEFAULT,
			sbody->GetMaxChildOrbitalDistance() * 1.1);
		Frame *orbFrame = Frame::GetFrame(orbFrameId);
		orbFrame->SetBodies(sbody, b);
		return orbFrameId;
	}

	SystemBody::BodySuperType supertype = sbody->GetSuperType();

	if ((supertype == SystemBody::SUPERTYPE_GAS_GIANT) ||
		(supertype == SystemBody::SUPERTYPE_ROCKY_PLANET)) {
		// for planets we want an non-rotating frame covering its Hill radius
		// and a rotating frame with no radius to contain attached objects
		double hillRadius = sbody->GetSemiMajorAxis() * (1.0 - sbody->GetEccentricity()) * pow(sbody->GetMass() / (3.0 * sbody->GetParent()->GetMass()), 1.0 / 3.0);
		double frameRadius = std::max(hillRadius, sbody->GetMaxChildOrbitalDistance() * 1.05);
		frameRadius = std::max(sbody->GetRadius() * 4.0, frameRadius);
		FrameId orbFrameId = Frame::CreateFrame(fId,
			sbody->GetName().c_str(),
			Frame::FLAG_HAS_ROT,
			frameRadius);
		Frame *orbFrame = Frame::GetFrame(orbFrameId);
		orbFrame->SetBodies(sbody, b);
		//Output("\t\t\t%s has frame size %.0fkm, body radius %.0fkm\n", sbody->name.c_str(),
		//	(frameRadius ? frameRadius : 10*sbody->GetRadius())*0.001f,
		//	sbody->GetRadius()*0.001f);

		assert(sbody->IsRotating() != 0);
		// rotating frame has atmosphere radius or feature height, whichever is larger
		FrameId rotFrameId = Frame::CreateFrame(orbFrameId,
			sbody->GetName().c_str(),
			Frame::FLAG_ROTATING,
			b->GetPhysRadius());
		Frame *rotFrame = Frame::GetFrame(rotFrameId);
		rotFrame->SetBodies(sbody, b);

		matrix3x3d rotMatrix = matrix3x3d::RotateX(sbody->GetAxialTilt());

		if (sbody->GetRotationPeriod() > 0.0) {
			double angSpeed = 2.0 * M_PI / sbody->GetRotationPeriod();
			rotFrame->SetAngSpeed(angSpeed);
		}

		if (sbody->HasRotationPhase())
			rotMatrix = rotMatrix * matrix3x3d::RotateY(sbody->GetRotationPhaseAtStart());
		rotFrame->SetInitialOrient(rotMatrix, at_time);

		b->SetFrame(rotFrameId);
		return orbFrameId;
	} else if (supertype == SystemBody::SUPERTYPE_STAR) {
		// stars want a single small non-rotating frame
		// bigger than it's furtherest orbiting body.
		// if there are no orbiting bodies use a frame of several radii.
		FrameId orbFrameId = Frame::CreateFrame(fId, sbody->GetName().c_str());
		Frame *orbFrame = Frame::GetFrame(orbFrameId);
		orbFrame->SetBodies(sbody, b);
		const double bodyRadius = sbody->GetEquatorialRadius();
		double frameRadius = std::max(10.0 * bodyRadius, sbody->GetMaxChildOrbitalDistance() * 1.1);
		// Respect the frame of other stars in the multi-star system. We still make sure that the frame ends outside
		// the body. For a minimum separation of 1.236 radii, nothing will overlap (see StarSystem::StarSystem()).
		if (sbody->GetParent() && frameRadius > AU * 0.11 * sbody->GetOrbMin())
			frameRadius = std::max(1.1 * bodyRadius, AU * 0.11 * sbody->GetOrbMin());
		orbFrame->SetRadius(frameRadius);
		b->SetFrame(orbFrameId);
		return orbFrameId;
	} else if (sbody->GetType() == SystemBody::TYPE_STARPORT_ORBITAL) {
		// space stations want non-rotating frame to some distance
		FrameId orbFrameId = Frame::CreateFrame(fId,
			sbody->GetName().c_str(),
			Frame::FLAG_DEFAULT,
			20000.0);
		Frame *orbFrame = Frame::GetFrame(orbFrameId);
		orbFrame->SetBodies(sbody, b);
		b->SetFrame(orbFrameId);
		return orbFrameId;
	} else if (sbody->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
		// just put body into rotating frame of planet, not in its own frame
		// (because collisions only happen between objects in same frame,
		// and we want collisions on starport and on planet itself)
		FrameId rotFrameId = Frame::GetFrame(fId)->GetRotFrame();
		b->SetFrame(rotFrameId);

		Frame *rotFrame = Frame::GetFrame(rotFrameId);
		assert(rotFrame->IsRotFrame());
		assert(rotFrame->GetBody()->IsType(ObjectType::PLANET));
		matrix3x3d rot;
		vector3d pos;
		Planet *planet = static_cast<Planet *>(rotFrame->GetBody());
		RelocateStarportIfNecessary(sbody, planet, pos, rot, prevPositions);
		sbody->SetOrbitPlane(rot);
		b->SetPosition(pos * planet->GetTerrainHeight(pos));
		b->SetOrient(rot);
		// accumulate for testing against
		prevPositions.push_back(pos);
		return rotFrameId;
	} else {
		assert(0);
	}
	return 0;
}

void Space::GenBody(const double at_time, SystemBody *sbody, FrameId fId, std::vector<vector3d> &posAccum)
{
	PROFILE_START()
	Body *b = nullptr;

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
		b->SetPosition(vector3d(0, 0, 0));
		AddBody(b);
	}
	fId = MakeFramesFor(at_time, sbody, b, fId, posAccum);

	PROFILE_STOP()
	for (SystemBody *kid : sbody->GetChildren()) {
		GenBody(at_time, kid, fId, posAccum);
	}
}

static bool OnCollision(Body *o1, Body *o2, CollisionContact *c, double relativeVel)
{
	/* XXX: if you create a new class inheriting from Object instead of Body, this code must be updated */
	if (o1 && !o1->OnCollision(o2, c->geomFlag, relativeVel)) return false;
	if (o2 && !o2->OnCollision(o1, c->geomFlag, relativeVel)) return false;
	return true;
}

static void hitCallback(CollisionContact *c)
{
	//Output("OUCH! %x (depth %f)\n", SDL_GetTicks(), c->depth);

	Body *po1 = static_cast<Body *>(c->userData1);
	Body *po2 = static_cast<Body *>(c->userData2);

	const bool po1_isDynBody = po1->IsType(ObjectType::DYNAMICBODY);
	const bool po2_isDynBody = po2->IsType(ObjectType::DYNAMICBODY);
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
	PROFILE_SCOPED()
	if (!body->IsType(ObjectType::DYNAMICBODY))
		return;
	DynamicBody *dynBody = static_cast<DynamicBody *>(body);
	if (!dynBody->IsMoving())
		return;

	Frame *f = Frame::GetFrame(body->GetFrame());
	if (!f || !f->GetBody() || f->GetId() != f->GetBody()->GetFrame())
		return;
	if (!f->GetBody()->IsType(ObjectType::TERRAINBODY))
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

void Space::TimeStep(float step)
{
	PROFILE_SCOPED()

	if (Pi::MustRefreshBackgroundClearFlag())
		RefreshBackground();

	m_bodyIndexValid = m_sbodyIndexValid = false;

	Frame::CollideFrames(&hitCallback);

	for (Body *b : m_bodies)
		CollideWithTerrain(b, step);

	// update frames of reference
	for (Body *b : m_bodies)
		b->UpdateFrame();

	// AI acts here, then move all bodies and frames
	// NOTE: The AI can add bodies here so we can't use an iterator
	// this restores the previous version where only the initial list is
	// updated unless the bodies vector reallocated where anything could
	// have happened
	// alternative fixes to delay addition caused
	// https://github.com/pioneerspacesim/pioneer/issues/5695
	//
	// THIS IS A HACK/WORKAROUND until a more proper solution can be found
	for (size_t i = 0; i < m_bodies.size(); ++i) {
		auto b = m_bodies[i];
		b->StaticUpdate(step);
	}
	Frame::UpdateOrbitRails(m_game->GetTime(), m_game->GetTimeStep());

	for (Body *b : m_bodies)
		b->TimeStepUpdate(step);

	LuaEvent::Emit();
	Pi::luaTimer->Tick();

	UpdateBodies();

	m_bodyNearFinder.Prepare();
}

void Space::UpdateBodies()
{
	PROFILE_SCOPED()
#ifndef NDEBUG
	m_processingFinalizationQueue = true;
#endif

	// removing or deleting bodies from space
	for (const auto &b : m_assignedBodies) {
		auto remove_iterator = m_bodies.end();
		for (auto it = m_bodies.begin(); it != m_bodies.end(); ++it) {
			if (*it != b.first)
				(*it)->NotifyRemoved(b.first);
			else
				remove_iterator = it;
		}
		if (remove_iterator != m_bodies.end()) {
			*remove_iterator = m_bodies.back();
			m_bodies.pop_back();
			if (b.second == BodyAssignation::KILL)
				delete b.first;
			else
				b.first->SetFrame(FrameId::Invalid);
		}
	}

	m_assignedBodies.clear();

#ifndef NDEBUG
	m_processingFinalizationQueue = false;
#endif
}

static char space[256];

static void DebugDumpFrame(FrameId fId, bool details, unsigned int indent)
{
	Frame *f = Frame::GetFrame(fId);
	Frame *parent = Frame::GetFrame(f->GetParent());

	Output("%.*s%2i) %p (%s)%s\n", indent, space, static_cast<int>(fId), static_cast<void *>(f), f->GetLabel().c_str(), f->IsRotFrame() ? " [rotating]" : " [non rotating]");
	if (f->GetParent().valid())
		Output("%.*s parent %p (%s)\n", indent + 3, space, static_cast<void *>(parent), parent->GetLabel().c_str());
	if (f->GetBody())
		Output("%.*s body %p (%s)\n", indent + 3, space, static_cast<void *>(f->GetBody()), f->GetBody()->GetLabel().c_str());
	if (Body *b = f->GetBody())
		Output("%.*s bodyFor %p (%s)\n", indent + 3, space, static_cast<void *>(b), b->GetLabel().c_str());
	Output("%.*s distance: %f radius: %f children: %u\n", indent + 3, space, f->GetPosition().Length(), f->GetRadius(), f->GetNumChildren());

	for (FrameId kid : f->GetChildren())
		DebugDumpFrame(kid, details, indent + 2);
}

void Space::DebugDumpFrames(bool details)
{
	memset(space, ' ', sizeof(space));

	if (m_starSystem)
		Output("Frame structure for '%s':\n", m_starSystem->GetName().c_str());
	else
		Output("Frame structure while in hyperspace:\n");
	DebugDumpFrame(m_rootFrameId, details, 3);
}
