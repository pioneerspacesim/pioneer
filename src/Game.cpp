// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Game.h"
#include "Factions.h"
#include "Space.h"
#include "Player.h"
#include "Body.h"
#include "SpaceStation.h"
#include "HyperspaceCloud.h"
#include "Pi.h"
#include "ShipCpanel.h"
#include "Sfx.h"
#include "MathUtil.h"
#include "SectorView.h"
#include "WorldView.h"
#include "DeathView.h"
#include "SystemView.h"
#include "SystemInfoView.h"
#include "UIView.h"
#include "LuaEvent.h"
#include "LuaRef.h"
#include "ObjectViewerView.h"
#include "FileSystem.h"
#include "graphics/Renderer.h"
#include "ui/Context.h"
#include "galaxy/GalaxyGenerator.h"

extern "C" {
#include "miniz/miniz.h"
}

static const int  s_saveVersion   = 84;
static const char s_saveStart[]   = "PIONEER";
static const char s_saveEnd[]     = "END";

Game::Game(const SystemPath &path, double time) :
	m_galaxy(GalaxyGenerator::Create()),
	m_time(time),
	m_state(STATE_NORMAL),
	m_wantHyperspace(false),
	m_timeAccel(TIMEACCEL_1X),
	m_requestedTimeAccel(TIMEACCEL_1X),
	m_forceTimeAccel(false)
{
	// Now that we have a Galaxy, check the starting location
	if (!path.IsBodyPath())
		throw InvalidGameStartLocation("SystemPath is not a body path");
	RefCountedPtr<const Sector> s = m_galaxy->GetSector(path);
	if (size_t(path.systemIndex) >= s->m_systems.size()) {
		char buf[128];
		std::sprintf(buf, "System %u in sector <%d,%d,%d> does not exist",
			unsigned(path.systemIndex), int(path.sectorX), int(path.sectorY), int(path.sectorZ));
		throw InvalidGameStartLocation(std::string(buf));
	}
	RefCountedPtr<StarSystem> sys = m_galaxy->GetStarSystem(path);
	if (path.bodyIndex >= sys->GetNumBodies()) {
		char buf[256];
		std::sprintf(buf, "Body %d in system <%d,%d,%d : %d ('%s')> does not exist", unsigned(path.bodyIndex),
			int(path.sectorX), int(path.sectorY), int(path.sectorZ), unsigned(path.systemIndex), sys->GetName().c_str());
		throw InvalidGameStartLocation(std::string(buf));
	}

	m_space.reset(new Space(this, m_galaxy, path));

	Body *b = m_space->FindBodyForPath(&path);
	assert(b);

	m_player.reset(new Player("kanara"));

	m_space->AddBody(m_player.get());

	m_player->SetFrame(b->GetFrame());

	if (b->GetType() == Object::SPACESTATION) {
		m_player->SetDockedWith(static_cast<SpaceStation*>(b), 0);
	} else {
		const SystemBody *sbody = b->GetSystemBody();
		m_player->SetPosition(vector3d(0, 1.5*sbody->GetRadius(), 0));
		m_player->SetVelocity(vector3d(0,0,0));
	}

	CreateViews();

	EmitPauseState(IsPaused());
}

Game::~Game()
{
	DestroyViews();

	// XXX this shutdown sequence is critical:
	// 1- RemoveBody marks the Player for removal from Space,
	// 2- Space is destroyed, which actually goes through its removal list,
	//    removes the Player from Space and calls SetFrame(0) on it, which unlinks
	//    any references it has to other Space items
	// 3- Player is destroyed
	//
	// note that because of the declaration order of m_space and m_player in Game,
	// without these explicit Reset() calls, m_player would be deleted before m_space is,
	// which causes problems because then when Space is destroyed it tries to reference
	// the deleted Player object (to call SetFrame(0))
	//
	// ideally we'd split Player out into two objects, the physics part that
	// is owned by the space, and the game part that holds all the player
	// attributes and whatever else

	m_space->RemoveBody(m_player.get());
	m_space.reset();
	m_player.reset();
	m_galaxy->FlushCaches();
}

Game::Game(const Json::Value &jsonObj) :
m_timeAccel(TIMEACCEL_PAUSED),
m_requestedTimeAccel(TIMEACCEL_PAUSED),
m_forceTimeAccel(false)
{
	// signature check
	if (!jsonObj.isMember("signature")) throw SavedGameCorruptException();
	Json::Value signature = jsonObj["signature"];
	if (signature.isString() && signature.asString().compare(s_saveStart) == 0) {}
	else throw SavedGameCorruptException();

	// version check
	if (!jsonObj.isMember("version")) throw SavedGameCorruptException();
	Json::Value version = jsonObj["version"];
	if (!version.isInt()) throw SavedGameCorruptException();
	Output("savefile version: %d\n", version.asInt());
	if (version.asInt() == s_saveVersion) {}
	else
	{
		Output("can't load savefile, expected version: %d\n", s_saveVersion);
		throw SavedGameWrongVersionException();
	}

	// Preparing the Lua stuff
	Pi::luaSerializer->InitTableRefs();

	// galaxy generator
	m_galaxy = Galaxy::LoadFromJson(jsonObj);

	// game state
	if (!jsonObj.isMember("time")) throw SavedGameCorruptException();
	if (!jsonObj.isMember("state")) throw SavedGameCorruptException();
	m_time = StrToDouble(jsonObj["time"].asString());
	m_state = State(jsonObj["state"].asInt());

	if (!jsonObj.isMember("want_hyperspace")) throw SavedGameCorruptException();
	if (!jsonObj.isMember("hyperspace_progress")) throw SavedGameCorruptException();
	if (!jsonObj.isMember("hyperspace_duration")) throw SavedGameCorruptException();
	if (!jsonObj.isMember("hyperspace_end_time")) throw SavedGameCorruptException();
	m_wantHyperspace = jsonObj["want_hyperspace"].asBool();
	m_hyperspaceProgress = StrToDouble(jsonObj["hyperspace_progress"].asString());
	m_hyperspaceDuration = StrToDouble(jsonObj["hyperspace_duration"].asString());
	m_hyperspaceEndTime = StrToDouble(jsonObj["hyperspace_end_time"].asString());

	// space, all the bodies and things
	if (!jsonObj.isMember("player")) throw SavedGameCorruptException();
	m_space.reset(new Space(this, m_galaxy, jsonObj, m_time));
	m_player.reset(static_cast<Player*>(m_space->GetBodyByIndex(jsonObj["player"].asUInt())));

	assert(!m_player->IsDead()); // Pioneer does not support necromancy

	// hyperspace clouds being brought over from the previous system
	if (!jsonObj.isMember("hyperspace_clouds")) throw SavedGameCorruptException();
	Json::Value hyperspaceCloudArray = jsonObj["hyperspace_clouds"];
	if (!hyperspaceCloudArray.isArray()) throw SavedGameCorruptException();
	for (Uint32 i = 0; i < hyperspaceCloudArray.size(); i++)
		m_hyperspaceClouds.push_back(static_cast<HyperspaceCloud*>(Body::FromJson(hyperspaceCloudArray[i], 0)));

	// views
	LoadViewsFromJson(jsonObj);

	// lua
	Pi::luaSerializer->FromJson(jsonObj);

	Pi::luaSerializer->UninitTableRefs();

	// signature check (don't really need this anymore)
	if (!jsonObj.isMember("trailing_signature")) throw SavedGameCorruptException();
	Json::Value trailingSignature = jsonObj["trailing_signature"];
	if (trailingSignature.isString() && trailingSignature.asString().compare(s_saveEnd) == 0) {}
	else throw SavedGameCorruptException();

	EmitPauseState(IsPaused());
}

void Game::ToJson(Json::Value &jsonObj)
{
	PROFILE_SCOPED()
	// preparing the lua serializer
	Pi::luaSerializer->InitTableRefs();

	// signature
	jsonObj["signature"] = s_saveStart;

	// version
	jsonObj["version"] = s_saveVersion;

	// galaxy generator
	m_galaxy->ToJson(jsonObj);

	// game state
	jsonObj["time"] = DoubleToStr(m_time);
	jsonObj["state"] = Uint32(m_state);

	jsonObj["want_hyperspace"] = m_wantHyperspace;
	jsonObj["hyperspace_progress"] = DoubleToStr(m_hyperspaceProgress);
	jsonObj["hyperspace_duration"] = DoubleToStr(m_hyperspaceDuration);
	jsonObj["hyperspace_end_time"] = DoubleToStr(m_hyperspaceEndTime);

	// space, all the bodies and things
	m_space->ToJson(jsonObj);
	jsonObj["player"] = m_space->GetIndexForBody(m_player.get());

	// hyperspace clouds being brought over from the previous system
	Json::Value hyperspaceCloudArray(Json::arrayValue); // Create JSON array to contain hyperspace cloud data.
	for (std::list<HyperspaceCloud*>::const_iterator i = m_hyperspaceClouds.begin(); i != m_hyperspaceClouds.end(); ++i)
	{
		Json::Value hyperspaceCloudArrayEl(Json::objectValue); // Create JSON object to contain hyperspace cloud.
		(*i)->ToJson(hyperspaceCloudArrayEl, m_space.get());
		hyperspaceCloudArray.append(hyperspaceCloudArrayEl); // Append hyperspace cloud object to array.
	}
	jsonObj["hyperspace_clouds"] = hyperspaceCloudArray; // Add hyperspace cloud array to supplied object.

	// views. must be saved in init order
	m_gameViews->m_cpan->SaveToJson(jsonObj);
	m_gameViews->m_sectorView->SaveToJson(jsonObj);
	m_gameViews->m_worldView->SaveToJson(jsonObj);

	// lua
	Pi::luaSerializer->ToJson(jsonObj);

	// trailing signature
	jsonObj["trailing_signature"] = s_saveEnd; // Don't really need this anymore.

	Pi::luaSerializer->UninitTableRefs();
}

void Game::TimeStep(float step)
{
	PROFILE_SCOPED()
	m_time += step;			// otherwise planets lag time accel changes by a frame
	if (m_state == STATE_HYPERSPACE && Pi::game->GetTime() >= m_hyperspaceEndTime)
		m_time = m_hyperspaceEndTime;

	m_space->TimeStep(step);

	// XXX ui updates, not sure if they belong here
	m_gameViews->m_cpan->TimeStepUpdate(step);
	SfxManager::TimeStepAll(step, m_space->GetRootFrame());

	if (m_state == STATE_HYPERSPACE) {
		if (Pi::game->GetTime() >= m_hyperspaceEndTime) {
			SwitchToNormalSpace();
			m_player->EnterSystem();
			RequestTimeAccel(TIMEACCEL_1X);
		}
		else
			m_hyperspaceProgress += step;
		return;
	}

	if (m_wantHyperspace) {
		assert(m_state == STATE_NORMAL);
		SwitchToHyperspace();
		return;
	}
}

bool Game::UpdateTimeAccel()
{
	PROFILE_SCOPED()
	// don't modify the timeaccel if the game is paused
	if (m_requestedTimeAccel == Game::TIMEACCEL_PAUSED) {
		SetTimeAccel(Game::TIMEACCEL_PAUSED);
		return false;
	}

	TimeAccel newTimeAccel = m_requestedTimeAccel;

	// ludicrous speed
	if (m_player->GetFlightState() == Ship::HYPERSPACE) {
		newTimeAccel = Game::TIMEACCEL_HYPERSPACE;
		RequestTimeAccel(newTimeAccel);
	}

	// force down to timeaccel 1 during the docking sequence or when just initiating hyperspace
	else if (m_player->GetFlightState() == Ship::DOCKING ||
			 m_player->GetFlightState() == Ship::JUMPING ||
			 m_player->GetFlightState() == Ship::UNDOCKING) {
		newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_10X);
		RequestTimeAccel(newTimeAccel);
	}

	// normal flight
	else if (m_player->GetFlightState() == Ship::FLYING) {

		// limit timeaccel to 1x when fired on (no forced acceleration allowed)
		if (m_player->GetAlertState() == Ship::ALERT_SHIP_FIRING)
			newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_1X);

		if (!m_forceTimeAccel) {

				// if not forced - limit timeaccel to 10x when other ships are close
			if (m_player->GetAlertState() == Ship::ALERT_SHIP_NEARBY)
				newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_10X);

				// if not forced - check if we aren't too near to objects for timeaccel
			else {
				for (const Body* b : m_space->GetBodies()) {
					if (b == m_player.get()) continue;
					if (b->IsType(Object::HYPERSPACECLOUD)) continue;

					vector3d toBody = m_player->GetPosition() - b->GetPositionRelTo(m_player->GetFrame());
					double dist = toBody.Length();
					double rad = b->GetPhysRadius();

					if (dist < 1000.0) {
						newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_1X);
					} else if (dist < std::min(rad+0.0001*AU, rad*1.1)) {
						newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_10X);
					} else if (dist < std::min(rad+0.001*AU, rad*5.0)) {
						newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_100X);
					} else if (dist < std::min(rad+0.01*AU,rad*10.0)) {
						newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_1000X);
					} else if (dist < std::min(rad+0.1*AU, rad*1000.0)) {
						newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_10000X);
					}
				}
			}

			if (!m_player->AIIsActive()) {		// don't do this when autopilot is active
				const double locVel = m_player->GetAngVelocity().Length();
				const double strictness = 20.0;
				if(locVel > strictness / Game::s_timeAccelRates[TIMEACCEL_10X]) {
					newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_1X);
				} else if(locVel > strictness / Game::s_timeAccelRates[TIMEACCEL_100X]) {
					newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_10X);
				} else if(locVel > strictness / Game::s_timeAccelRates[TIMEACCEL_1000X]) {
					newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_100X);
				} else if(locVel > strictness / Game::s_timeAccelRates[TIMEACCEL_10000X]) {
					newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_1000X);
				}
			}
		}
	}

	// no change
	if (newTimeAccel == m_timeAccel)
		return false;

	SetTimeAccel(newTimeAccel);
	return true;
}

void Game::WantHyperspace()
{
	assert(m_state == STATE_NORMAL);
	m_wantHyperspace = true;
}

double Game::GetHyperspaceArrivalProbability() const
{
	double progress = m_hyperspaceProgress / m_hyperspaceDuration;
	const double fudge = 4.0;
	const double scale = 1.0 / (1.0 - exp(-fudge));
	return scale * (1.0 - exp(-fudge * progress));
}

void Game::RemoveHyperspaceCloud(HyperspaceCloud* cloud)
{
	m_hyperspaceClouds.remove(cloud);
}

void Game::SwitchToHyperspace()
{
	PROFILE_SCOPED()
	// remember where we came from so we can properly place the player on exit
	m_hyperspaceSource = m_space->GetStarSystem()->GetPath();
	m_hyperspaceDest =  m_player->GetHyperspaceDest();

	// find all the departure clouds, convert them to arrival clouds and store
	// them for the next system
	m_hyperspaceClouds.clear();
	for (Body* b : m_space->GetBodies()) {

		if (!b->IsType(Object::HYPERSPACECLOUD)) continue;

		// only want departure clouds with ships in them
		HyperspaceCloud *cloud = static_cast<HyperspaceCloud*>(b);
		if (cloud->IsArrival() || cloud->GetShip() == 0)
			continue;

		// make sure they're going to the same place as us
		if (!m_hyperspaceDest.IsSameSystem(cloud->GetShip()->GetHyperspaceDest()))
			continue;

		// remove it from space
		m_space->RemoveBody(cloud);

		// player and the clouds are coming to the next system, but we don't
		// want the player to have any memory of what they were (we're just
		// reusing them for convenience). tell the player it was deleted so it
		// can clean up
		m_player->NotifyRemoved(cloud);

		// turn the cloud arround
		cloud->GetShip()->SetHyperspaceDest(m_hyperspaceSource);
		cloud->SetIsArrival(true);

		// and remember it
		m_hyperspaceClouds.push_back(cloud);
	}

	Output(SIZET_FMT " clouds brought over\n", m_hyperspaceClouds.size());

	// remove the player from space
	m_space->RemoveBody(m_player.get());

	// create hyperspace :)
	m_space.reset(new Space(this, m_galaxy, m_space.get()));

	m_space->GetBackground()->SetDrawFlags( Background::Container::DRAW_STARS );

	// Reset planner
	Pi::planner->ResetStartTime();
	Pi::planner->ResetDv();

	// put the player in it
	m_player->SetFrame(m_space->GetRootFrame());
	m_space->AddBody(m_player.get());

	// put player at the origin. kind of unnecessary since it won't be moving
	// but at least it gives some consistency
	m_player->SetPosition(vector3d(0,0,0));
	m_player->SetVelocity(vector3d(0,0,0));
	m_player->SetOrient(matrix3x3d::Identity());

	// animation and end time counters
	m_hyperspaceProgress = 0;
	m_hyperspaceDuration = m_player->GetHyperspaceDuration();
	m_hyperspaceEndTime = Pi::game->GetTime() + m_hyperspaceDuration;

	m_state = STATE_HYPERSPACE;
	m_wantHyperspace = false;

	Output("Started hyperspacing...\n");
}

void Game::SwitchToNormalSpace()
{
	PROFILE_SCOPED()
	// remove the player from hyperspace
	m_space->RemoveBody(m_player.get());

	// create a new space for the system
	m_space.reset(new Space(this, m_galaxy, m_hyperspaceDest, m_space.get()));

	// put the player in it
	m_player->SetFrame(m_space->GetRootFrame());
	m_space->AddBody(m_player.get());

	// place it
	m_player->SetPosition(m_space->GetHyperspaceExitPoint(m_hyperspaceSource, m_hyperspaceDest));
	m_player->SetVelocity(vector3d(0,0,-100.0));
	m_player->SetOrient(matrix3x3d::Identity());

	// place the exit cloud
	HyperspaceCloud *cloud = new HyperspaceCloud(0, Pi::game->GetTime(), true);
	cloud->SetFrame(m_space->GetRootFrame());
	cloud->SetPosition(m_player->GetPosition());
	m_space->AddBody(cloud);

	for (std::list<HyperspaceCloud*>::iterator i = m_hyperspaceClouds.begin(); i != m_hyperspaceClouds.end(); ++i) {
		cloud = *i;

		cloud->SetFrame(m_space->GetRootFrame());
		cloud->SetPosition(m_space->GetHyperspaceExitPoint(m_hyperspaceSource, m_hyperspaceDest));

		m_space->AddBody(cloud);

		if (cloud->GetDueDate() < Pi::game->GetTime()) {
			// they emerged from hyperspace some time ago
			Ship *ship = cloud->EvictShip();

			ship->SetFrame(m_space->GetRootFrame());
			ship->SetVelocity(vector3d(0,0,-100.0));
			ship->SetOrient(matrix3x3d::Identity());
			ship->SetFlightState(Ship::FLYING);

			const SystemPath &sdest = ship->GetHyperspaceDest();
			if (sdest.IsSystemPath()) {
				// travelling to the system as a whole, so just dump them on
				// the cloud - we can't do any better in this case
				ship->SetPosition(cloud->GetPosition());
			}

			else {
				// on their way to a body. they're already in-system so we
				// want to simulate some travel to their destination. we
				// naively assume full accel for half the distance, flip and
				// full brake for the rest.
				Body *target_body = m_space->FindBodyForPath(&sdest);
				double dist_to_target = cloud->GetPositionRelTo(target_body).Length();
				double half_dist_to_target = dist_to_target / 2.0;
				//double accel = -(ship->GetShipType()->linThrust[ShipType::THRUSTER_FORWARD] / ship->GetMass());
				double accel = -ship->GetAccelFwd();
				double travel_time = Pi::game->GetTime() - cloud->GetDueDate();

				// I can't help but feel some actual math would do better here
				double speed = 0;
				double dist = 0;
				while (travel_time > 0 && dist <= half_dist_to_target) {
					speed += accel;
					dist += speed;
					travel_time--;
				}
				while (travel_time > 0 && dist < dist_to_target) {
					speed -= accel;
					dist += speed;
					travel_time--;
				}

				if (travel_time <= 0) {
					vector3d pos =
						target_body->GetPositionRelTo(m_space->GetRootFrame()) +
						cloud->GetPositionRelTo(target_body).Normalized() * (dist_to_target - dist);
					ship->SetPosition(pos);
				}

				else {
					// ship made it with time to spare. just put it somewhere
					// near the body. the script should be issuing a dock or
					// flyto command in onEnterSystem so it should sort it
					// itself out long before the player can get near

					SystemBody *sbody = m_space->GetStarSystem()->GetBodyByPath(&sdest);
					if (sbody->GetType() == SystemBody::TYPE_STARPORT_ORBITAL) {
						ship->SetFrame(target_body->GetFrame());
						ship->SetPosition(MathUtil::RandomPointOnSphere(1000.0)*1000.0); // somewhere 1000km out
					}

					else {
						if (sbody->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
							sbody = sbody->GetParent();
							SystemPath path = m_space->GetStarSystem()->GetPathOf(sbody);
							target_body = m_space->FindBodyForPath(&path);
						}

						double sdist = sbody->GetRadius()*2.0;

						ship->SetFrame(target_body->GetFrame());
						ship->SetPosition(MathUtil::RandomPointOnSphere(sdist));
					}
				}
			}

			m_space->AddBody(ship);

			LuaEvent::Queue("onEnterSystem", ship);
		}
	}
	m_hyperspaceClouds.clear();

	m_space->GetBackground()->SetDrawFlags( Background::Container::DRAW_SKYBOX | Background::Container::DRAW_STARS );

	m_state = STATE_NORMAL;
}

const float Game::s_timeAccelRates[] = {
	0.0f,       // paused
	1.0f,       // 1x
	10.0f,      // 10x
	100.0f,     // 100x
	1000.0f,    // 1000x
	10000.0f,   // 10000x
	100000.0f   // hyperspace
};

const float Game::s_timeInvAccelRates[] = {
	0.0f,       // paused
	1.0f,       // 1x
	0.1f,      // 10x
	0.01f,     // 100x
	0.001f,    // 1000x
	0.0001f,   // 10000x
	0.00001f   // hyperspace
};

void Game::SetTimeAccel(TimeAccel t)
{
	// don't want player to spin like mad when hitting time accel
	if ((t != m_timeAccel) && (t > TIMEACCEL_1X) &&
			m_player->GetPlayerController()->GetRotationDamping()) {
		m_player->SetAngVelocity(vector3d(0,0,0));
		m_player->SetTorque(vector3d(0,0,0));
		m_player->SetAngThrusterState(vector3d(0.0));
	}

	// Give all ships a half-step acceleration to stop autopilot overshoot
	if (t < m_timeAccel)
		for (Body* b : m_space->GetBodies())
			if (b->IsType(Object::SHIP))
				(static_cast<Ship*>(b))->TimeAccelAdjust(0.5f * GetTimeStep());

	bool emitPaused = (t == TIMEACCEL_PAUSED && t != m_timeAccel);
	bool emitResumed = (m_timeAccel == TIMEACCEL_PAUSED && t != TIMEACCEL_PAUSED);

	m_timeAccel = t;

	if (emitPaused) {
		EmitPauseState(true);
	}

	if (emitResumed) {
		EmitPauseState(false);
	}

	if (m_timeAccel == TIMEACCEL_PAUSED || m_timeAccel == TIMEACCEL_HYPERSPACE) {
		m_requestedTimeAccel = m_timeAccel;
		m_forceTimeAccel = false;
	}
}

void Game::RequestTimeAccel(TimeAccel t, bool force)
{
	m_requestedTimeAccel = t;
	m_forceTimeAccel = force;
}

void Game::RequestTimeAccelInc(bool force)
{
    switch(m_requestedTimeAccel) {
        case Game::TIMEACCEL_1X:
            m_requestedTimeAccel = Game::TIMEACCEL_10X;
            break;
        case Game::TIMEACCEL_10X:
            m_requestedTimeAccel = Game::TIMEACCEL_100X;
            break;
        case Game::TIMEACCEL_100X:
            m_requestedTimeAccel = Game::TIMEACCEL_1000X;
            break;
        case Game::TIMEACCEL_1000X:
            m_requestedTimeAccel = Game::TIMEACCEL_10000X;
            break;
        default:
            // ignore if paused, hyperspace or 10000X
            break;
    }
    m_forceTimeAccel = force;
}

void Game::RequestTimeAccelDec(bool force)
{
    switch(m_requestedTimeAccel) {
        case Game::TIMEACCEL_10X:
            m_requestedTimeAccel = Game::TIMEACCEL_1X;
            break;
        case Game::TIMEACCEL_100X:
            m_requestedTimeAccel = Game::TIMEACCEL_10X;
            break;
        case Game::TIMEACCEL_1000X:
            m_requestedTimeAccel = Game::TIMEACCEL_100X;
            break;
        case Game::TIMEACCEL_10000X:
            m_requestedTimeAccel = Game::TIMEACCEL_1000X;
            break;
        default:
            // ignore if paused, hyperspace or 1X
            break;
    }
    m_forceTimeAccel = force;
}

Game::Views::Views()
	: m_sectorView(nullptr)
	, m_galacticView(nullptr)
	, m_settingsView(nullptr)
	, m_systemInfoView(nullptr)
	, m_systemView(nullptr)
	, m_worldView(nullptr)
	, m_deathView(nullptr)
	, m_spaceStationView(nullptr)
	, m_infoView(nullptr)
	, m_cpan(nullptr)
{ }

void Game::Views::SetRenderer(Graphics::Renderer *r)
{
	// view manager will handle setting this probably
	m_infoView->SetRenderer(r);
	m_sectorView->SetRenderer(r);
	m_systemInfoView->SetRenderer(r);
	m_systemView->SetRenderer(r);
	m_worldView->SetRenderer(r);
	m_deathView->SetRenderer(r);

#if WITH_OBJECTVIEWER
	m_objectViewerView->SetRenderer(r);
#endif
}

void Game::Views::Init(Game* game)
{
	m_cpan = new ShipCpanel(Pi::renderer, game);
	m_sectorView = new SectorView(game);
	m_worldView = new WorldView(game);
	m_galacticView = new UIView("GalacticView");
	m_systemView = new SystemView(game);
	m_systemInfoView = new SystemInfoView(game);
	m_spaceStationView = new UIView("StationView");
	m_infoView = new UIView("InfoView");
	m_deathView = new DeathView(game);
	m_settingsView = new UIView("SettingsInGame");

#if WITH_OBJECTVIEWER
	m_objectViewerView = new ObjectViewerView();
#endif

	SetRenderer(Pi::renderer);
}

void Game::Views::LoadFromJson(const Json::Value &jsonObj, Game* game)
{
	m_cpan = new ShipCpanel(jsonObj, Pi::renderer, game);
	m_sectorView = new SectorView(jsonObj, game);
	m_worldView = new WorldView(jsonObj, game);

	m_galacticView = new UIView("GalacticView");
	m_systemView = new SystemView(game);
	m_systemInfoView = new SystemInfoView(game);
	m_spaceStationView = new UIView("StationView");
	m_infoView = new UIView("InfoView");
	m_deathView = new DeathView(game);
	m_settingsView = new UIView("SettingsInGame");

#if WITH_OBJECTVIEWER
	m_objectViewerView = new ObjectViewerView();
#endif

	SetRenderer(Pi::renderer);
}

Game::Views::~Views()
{
#if WITH_OBJECTVIEWER
	delete m_objectViewerView;
#endif

	delete m_settingsView;
	delete m_deathView;
	delete m_infoView;
	delete m_spaceStationView;
	delete m_systemInfoView;
	delete m_systemView;
	delete m_galacticView;
	delete m_worldView;
	delete m_sectorView;
	delete m_cpan;
}

// XXX this should be in some kind of central UI management class that
// creates a set of UI views held by the game. right now though the views
// are rather fundamentally tied to their global points and assume they
// can all talk to each other. given the difficulty of disentangling all
// that and the impending move to Rocket, its better right now to just
// manage creation and destruction here to get the timing and order right
void Game::CreateViews()
{
	Pi::SetView(0);

	// XXX views expect Pi::game and Pi::player to exist
	Pi::game = this;
	Pi::player = m_player.get();

	m_gameViews.reset(new Views);
	m_gameViews->Init(this);

	UI::Point scrSize = Pi::ui->GetContext()->GetSize();
	log = new GameLog();
}

// XXX mostly a copy of CreateViews
void Game::LoadViewsFromJson(const Json::Value &jsonObj)
{
	Pi::SetView(0);

	// XXX views expect Pi::game and Pi::player to exist
	Pi::game = this;
	Pi::player = m_player.get();

	m_gameViews.reset(new Views);
	m_gameViews->LoadFromJson(jsonObj, this);

	UI::Point scrSize = Pi::ui->GetContext()->GetSize();
	log = new GameLog();
}

void Game::DestroyViews()
{
	Pi::SetView(0);

	m_gameViews.reset();

	delete log;
	log = 0;
}

void Game::EmitPauseState(bool paused)
{
	if (paused)	{
		// Notify UI that time is paused.
		LuaEvent::Queue("onGamePaused");
	} else {
		// Notify the UI that time is running again.
		LuaEvent::Queue("onGameResumed");
	}
	LuaEvent::Emit();
}

Game *Game::LoadGame(const std::string &filename)
{
	Output("Game::LoadGame('%s')\n", filename.c_str());
	auto file = FileSystem::userFiles.ReadFile(FileSystem::JoinPathBelow(Pi::SAVE_DIR_NAME, filename));
	if (!file) throw CouldNotOpenFileException();
	Json::Value rootNode; // Create the root JSON value for receiving the game data.
	Json::Reader jsonReader; // Create reader for parsing the JSON string.
	const auto data = file->AsByteRange();
	size_t outSize = 0;
	void *pDecompressedData = tinfl_decompress_mem_to_heap(&data[0], data.Size(), &outSize, 0);
	if (pDecompressedData) {
		jsonReader.parse(static_cast<char*>(pDecompressedData), static_cast<char*>(pDecompressedData)+outSize, rootNode); // Parse the JSON string.
		mz_free(pDecompressedData);
		if (!rootNode.isObject()) throw SavedGameCorruptException();
		return new Game(rootNode); // Decode the game data from JSON and create the game.
	} else {
		throw SavedGameCorruptException();
	}
	// file data is freed here
}

bool Game::CanLoadGame(const std::string &filename)
{
	Output("Game::CanLoadGame('%s')\n", filename.c_str());
	auto file = FileSystem::userFiles.ReadFile(FileSystem::JoinPathBelow(Pi::SAVE_DIR_NAME, filename));
	if (!file)
		return false;

	return true;
	// file data is freed here
}

void Game::SaveGame(const std::string &filename, Game *game)
{
	PROFILE_SCOPED()
	assert(game);

	if (game->IsHyperspace())
		throw CannotSaveInHyperspace();

	if (game->GetPlayer()->IsDead())
		throw CannotSaveDeadPlayer();

	if (!FileSystem::userFiles.MakeDirectory(Pi::SAVE_DIR_NAME)) {
		throw CouldNotOpenFileException();
	}

#ifdef PIONEER_PROFILER
	std::string profilerPath;
	FileSystem::userFiles.MakeDirectory("profiler");
	FileSystem::userFiles.MakeDirectory("profiler/saving");
	profilerPath = FileSystem::JoinPathBelow(FileSystem::userFiles.GetRoot(), "profiler/saving");
	Profiler::reset();
#endif

	Json::Value rootNode; // Create the root JSON value for receiving the game data.
	game->ToJson(rootNode); // Encode the game data as JSON and give to the root value.
	Json::FastWriter jsonWriter; // Create writer for writing the JSON data to string.
	const std::string jsonDataStr = jsonWriter.write(rootNode); // Write the JSON data.

	FILE *f = FileSystem::userFiles.OpenWriteStream(FileSystem::JoinPathBelow(Pi::SAVE_DIR_NAME, filename));
	if (!f) throw CouldNotOpenFileException();

	// compress in memory, write to open file
	size_t outSize = 0;
	void *pCompressedData = tdefl_compress_mem_to_heap(jsonDataStr.data(), jsonDataStr.length(), &outSize, 128);
	if (pCompressedData)
	{
		size_t nwritten = fwrite(pCompressedData, outSize, 1, f);
		mz_free(pCompressedData);
		fclose(f);
		if (nwritten != 1) throw CouldNotWriteToFileException();
	}
	else
	{
		fclose(f);
		throw CouldNotWriteToFileException();
	}

#ifdef PIONEER_PROFILER
	Profiler::dumphtml(profilerPath.c_str());
#endif
}
