// Copyright © 2008-2023 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "buildopts.h"

#include "Game.h"

#include "Body.h"
#include "DeathView.h"
#include "FileSystem.h"
#include "GameLog.h"
#include "GameSaveError.h"
#include "HyperspaceCloud.h"
#include "MathUtil.h"
#include "collider/CollisionSpace.h"
#include "core/GZipFormat.h"
#include "galaxy/Economy.h"
#include "lua/LuaEvent.h"
#include "lua/LuaSerializer.h"
#if WITH_OBJECTVIEWER
#include "ObjectViewerView.h"
#endif
#include "Pi.h"
#include "Player.h"
#include "SectorView.h"
#include "Sfx.h"
#include "Space.h"
#include "SpaceStation.h"
#include "SystemView.h"
#include "WorldView.h"
#include "galaxy/GalaxyGenerator.h"
#include "pigui/PiGuiView.h"
#include "ship/PlayerShipController.h"

static const int s_saveVersion = 88;

Game::Game(const SystemPath &path, const double startDateTime) :
	m_galaxy(GalaxyGenerator::Create()),
	m_time(startDateTime),
	m_state(State::NORMAL),
	m_wantHyperspace(false),
	m_hyperspaceProgress(0),
	m_hyperspaceDuration(0),
	m_hyperspaceEndTime(0),
	m_timeAccel(TIMEACCEL_1X),
	m_requestedTimeAccel(TIMEACCEL_1X),
	m_forceTimeAccel(false)
{
	PROFILE_SCOPED()
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

	if (b->GetType() == ObjectType::SPACESTATION) {
		m_player->SetDockedWith(static_cast<SpaceStation *>(b), 0);
	} else {
		const SystemBody *sbody = b->GetSystemBody();
		m_player->SetPosition(vector3d(0, 1.5 * sbody->GetRadius(), 0));
		m_player->SetVelocity(vector3d(0, 0, 0));
	}

	CreateViews();

	EmitPauseState(IsPaused());

	Pi::GetApp()->RequestProfileFrame("NewGame");
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

Game::Game(const Json &jsonObj) :
	m_timeAccel(TIMEACCEL_PAUSED),
	m_requestedTimeAccel(TIMEACCEL_PAUSED),
	m_forceTimeAccel(false)
{
	PROFILE_SCOPED()
	try {
		int version = jsonObj["version"];
		Output("savefile version: %d\n", version);
		if (version != s_saveVersion) {
			Output("can't load savefile, expected version: %d\n", s_saveVersion);
			throw SavedGameWrongVersionException();
		}
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	// Preparing the Lua stuff
	Pi::luaSerializer->InitTableRefs();

	GalacticEconomy::LoadFromJson(jsonObj);

	// galaxy generator
	m_galaxy = Galaxy::LoadFromJson(jsonObj);

	try {
		// game state
		m_time = jsonObj["time"];
		m_state = jsonObj["state"].get<State>();

		m_wantHyperspace = jsonObj["want_hyperspace"];
		m_hyperspaceProgress = jsonObj["hyperspace_progress"];
		m_hyperspaceDuration = jsonObj["hyperspace_duration"];
		m_hyperspaceEndTime = jsonObj["hyperspace_end_time"];

		// space, all the bodies and things
		m_space.reset(new Space(this, m_galaxy, jsonObj, m_time));

		unsigned int player = jsonObj["player"];
		m_player.reset(static_cast<Player *>(m_space->GetBodyByIndex(player)));

		assert(!m_player->IsDead()); // Pioneer does not support necromancy

		// hyperspace clouds being brought over from the previous system
		Json hyperspaceCloudArray = jsonObj["hyperspace_clouds"].get<Json::array_t>();
		for (Uint32 i = 0; i < hyperspaceCloudArray.size(); i++) {
			m_hyperspaceClouds.push_back(static_cast<HyperspaceCloud *>(Body::FromJson(hyperspaceCloudArray[i], 0)));
		}
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	// views
	LoadViewsFromJson(jsonObj);

	// lua components
	// the contents of m_space->m_bodies must not change until after this call
	Pi::luaSerializer->LoadComponents(jsonObj, m_space.get());

	// lua
	Pi::luaSerializer->FromJson(jsonObj);

	Pi::luaSerializer->UninitTableRefs();

	EmitPauseState(IsPaused());

	Pi::GetApp()->RequestProfileFrame("LoadGame");
}

void Game::ToJson(Json &jsonObj)
{
	PROFILE_SCOPED()
	// preparing the lua serializer
	Pi::luaSerializer->InitTableRefs();

	// version
	jsonObj["version"] = s_saveVersion;

	// galaxy generator
	m_galaxy->ToJson(jsonObj);

	GalacticEconomy::SaveToJson(jsonObj);

	// game state
	jsonObj["time"] = m_time;
	jsonObj["state"] = Uint32(m_state);

	jsonObj["want_hyperspace"] = m_wantHyperspace;
	jsonObj["hyperspace_progress"] = m_hyperspaceProgress;
	jsonObj["hyperspace_duration"] = m_hyperspaceDuration;
	jsonObj["hyperspace_end_time"] = m_hyperspaceEndTime;

	// Delete camera frame from frame structure:
	bool have_cam_frame = m_gameViews->m_worldView->GetCameraContext()->GetTempFrame().valid();
	if (have_cam_frame) m_gameViews->m_worldView->EndCameraFrame();

	// space, all the bodies and things
	m_space->ToJson(jsonObj);
	jsonObj["player"] = m_space->GetIndexForBody(m_player.get());

	// hyperspace clouds being brought over from the previous system
	Json hyperspaceCloudArray = Json::array(); // Create JSON array to contain hyperspace cloud data.
	for (std::list<HyperspaceCloud *>::const_iterator i = m_hyperspaceClouds.begin(); i != m_hyperspaceClouds.end(); ++i) {
		Json hyperspaceCloudArrayEl = Json::object(); // Create JSON object to contain hyperspace cloud.
		(*i)->ToJson(hyperspaceCloudArrayEl, m_space.get());
		hyperspaceCloudArray.push_back(hyperspaceCloudArrayEl); // Append hyperspace cloud object to array.
	}
	jsonObj["hyperspace_clouds"] = hyperspaceCloudArray; // Add hyperspace cloud array to supplied object.

	// views. must be saved in init order
	m_gameViews->m_sectorView->SaveToJson(jsonObj);
	m_gameViews->m_worldView->SaveToJson(jsonObj);

	// lua components
	// the contents of m_space->m_bodies must not change until after this call
	Pi::luaSerializer->SaveComponents(jsonObj, m_space.get());

	// lua
	Pi::luaSerializer->ToJson(jsonObj);

	// Stuff to show in the preview in load game window
	// some may be redundant, but this won't require loading up a game to get it all
	Json gameInfo = Json::object();
	float credits = LuaObject<Player>::CallMethod<float>(Pi::player, "GetMoney");

	gameInfo["system"] = Pi::game->GetSpace()->GetStarSystem()->GetName();
	gameInfo["credits"] = credits;
	gameInfo["ship"] = Pi::player->GetShipType()->modelName;
	if (Pi::player->IsDocked()) {
		gameInfo["docked_at"] = Pi::player->GetDockedWith()->GetSystemBody()->GetName();
	}

	switch (Pi::player->GetFlightState()) {
	case Ship::FlightState::DOCKED:
		gameInfo["flight_state"] = "docked";
		break;
	case Ship::FlightState::DOCKING:
		gameInfo["flight_state"] = "docking";
		break;
	case Ship::FlightState::FLYING:
		gameInfo["flight_state"] = "flying";
		break;
	case Ship::FlightState::HYPERSPACE:
		gameInfo["flight_state"] = "hyperspace";
		break;
	case Ship::FlightState::JUMPING:
		gameInfo["flight_state"] = "jumping";
		break;
	case Ship::FlightState::LANDED:
		gameInfo["flight_state"] = "landed";
		break;
	case Ship::FlightState::UNDOCKING:
		gameInfo["flight_state"] = "undocking";
		break;
	default:
		gameInfo["flight_state"] = "unknown";
		break;
	}

	jsonObj["game_info"] = gameInfo;

	Pi::luaSerializer->UninitTableRefs();

	// Bring back camera frame:
	if (have_cam_frame) m_gameViews->m_worldView->BeginCameraFrame();
}

void Game::TimeStep(float step)
{
	PROFILE_SCOPED()
	m_time += step; // otherwise planets lag time accel changes by a frame
	if (m_state == State::HYPERSPACE && Pi::game->GetTime() >= m_hyperspaceEndTime)
		m_time = m_hyperspaceEndTime;

	m_space->TimeStep(step);

	SfxManager::TimeStepAll(step, m_space->GetRootFrame());

	if (m_state == State::HYPERSPACE) {
		if (Pi::game->GetTime() >= m_hyperspaceEndTime) {
			SwitchToNormalSpace();
			m_player->EnterSystem();
			RequestTimeAccel(TIMEACCEL_1X);
		} else
			m_hyperspaceProgress += step;
		return;
	}

	if (m_wantHyperspace) {
		assert(m_state == State::NORMAL);
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
				for (const Body *b : m_space->GetBodies()) {
					if (b == m_player.get()) continue;
					if (b->IsType(ObjectType::HYPERSPACECLOUD)) continue;

					vector3d toBody = m_player->GetPosition() - b->GetPositionRelTo(m_player->GetFrame());
					double dist = toBody.Length();
					double rad = b->GetPhysRadius();

					if (dist < 1000.0) {
						newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_1X);
					} else if (dist < std::min(rad + 0.0001 * AU, rad * 1.1)) {
						newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_10X);
					} else if (dist < std::min(rad + 0.001 * AU, rad * 5.0)) {
						newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_100X);
					} else if (dist < std::min(rad + 0.01 * AU, rad * 10.0)) {
						newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_1000X);
					} else if (dist < std::min(rad + 0.1 * AU, rad * 1000.0)) {
						newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_10000X);
					}
				}
			}

			if (!m_player->AIIsActive()) { // don't do this when autopilot is active
				const double locVel = m_player->GetAngVelocity().Length();
				const double strictness = 20.0;
				if (locVel > strictness / Game::s_timeAccelRates[TIMEACCEL_10X]) {
					newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_1X);
				} else if (locVel > strictness / Game::s_timeAccelRates[TIMEACCEL_100X]) {
					newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_10X);
				} else if (locVel > strictness / Game::s_timeAccelRates[TIMEACCEL_1000X]) {
					newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_100X);
				} else if (locVel > strictness / Game::s_timeAccelRates[TIMEACCEL_10000X]) {
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
	assert(m_state == State::NORMAL);
	m_wantHyperspace = true;
}

double Game::GetHyperspaceArrivalProbability() const
{
	double progress = m_hyperspaceProgress / m_hyperspaceDuration;
	const double fudge = 4.0;
	const double scale = 1.0 / (1.0 - exp(-fudge));
	return scale * (1.0 - exp(-fudge * progress));
}

void Game::RemoveHyperspaceCloud(HyperspaceCloud *cloud)
{
	m_hyperspaceClouds.remove(cloud);
}

void Game::SwitchToHyperspace()
{
	PROFILE_SCOPED()
	// remember where we came from so we can properly place the player on exit
	m_hyperspaceSource = m_space->GetStarSystem()->GetPath();
	m_hyperspaceDest = m_player->GetHyperspaceDest();

	// find all the departure clouds, convert them to arrival clouds and store
	// them for the next system
	m_hyperspaceClouds.clear();
	for (Body *b : m_space->GetBodies()) {

		if (!b->IsType(ObjectType::HYPERSPACECLOUD)) continue;

		// only want departure clouds with ships in them
		HyperspaceCloud *cloud = static_cast<HyperspaceCloud *>(b);
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

		// turn the cloud around
		cloud->GetShip()->SetHyperspaceDest(m_hyperspaceSource);
		cloud->SetIsArrival(true);

		// and remember it
		m_hyperspaceClouds.push_back(cloud);
	}

	Output(SIZET_FMT " clouds brought over\n", m_hyperspaceClouds.size());

	// remove the player from space
	m_space->RemoveBody(m_player.get());

	// create hyperspace :)
	m_space.reset(); // HACK: Here because next line will create Frames *before* deleting existing ones
	m_space.reset(new Space(this, m_galaxy, m_space.get()));

	m_space->GetBackground()->SetDrawFlags(Background::Container::DRAW_STARS);

	// Reset planner
	Pi::planner->ResetStartTime();
	Pi::planner->ResetDv();

	// put the player in it
	m_player->SetFrame(m_space->GetRootFrame());
	m_space->AddBody(m_player.get());

	// put player at the origin. kind of unnecessary since it won't be moving
	// but at least it gives some consistency
	m_player->SetPosition(vector3d(0, 0, 0));
	m_player->SetVelocity(vector3d(0, 0, 0));
	m_player->SetOrient(matrix3x3d::Identity());

	// animation and end time counters
	m_hyperspaceProgress = 0;
	m_hyperspaceDuration = m_player->GetHyperspaceDuration();
	m_hyperspaceEndTime = Pi::game->GetTime() + m_hyperspaceDuration;

	m_state = State::HYPERSPACE;
	m_wantHyperspace = false;

	Output("Started hyperspacing...\n");
}

void Game::SwitchToNormalSpace()
{
	PROFILE_SCOPED()
	// remove the player from hyperspace
	m_space->RemoveBody(m_player.get());

	// create a new space for the system
	m_space.reset(); // HACK: Here because next line will create Frames *before* deleting existing ones
	m_space.reset(new Space(this, m_galaxy, m_hyperspaceDest, m_space.get()));
	m_state = State::NORMAL;

	// put the player in it
	m_player->SetFrame(m_space->GetRootFrame());
	m_space->AddBody(m_player.get());

	// place it
	vector3d pos, vel;
	m_space->GetHyperspaceExitParams(m_hyperspaceSource, m_hyperspaceDest, pos, vel);
	m_player->SetPosition(pos);
	m_player->SetVelocity(vel);

	// orient ship in direction of travel
	{
		vector3d oz = -vel.Normalized();
		vector3d ox = MathUtil::OrthogonalDirection(vel);
		vector3d oy = oz.Cross(ox).Normalized();
		m_player->SetOrient(matrix3x3d::FromVectors(ox, oy, oz));
	}

	// place the exit cloud
	HyperspaceCloud *cloud = new HyperspaceCloud(0, Pi::game->GetTime(), true);
	cloud->SetFrame(m_space->GetRootFrame());
	cloud->SetPosition(m_player->GetPosition());
	m_space->AddBody(cloud);

	for (std::list<HyperspaceCloud *>::iterator i = m_hyperspaceClouds.begin(); i != m_hyperspaceClouds.end(); ++i) {
		cloud = *i;

		cloud->SetFrame(m_space->GetRootFrame());
		cloud->SetPosition(m_space->GetHyperspaceExitPoint(m_hyperspaceSource, m_hyperspaceDest));

		m_space->AddBody(cloud);

		if (cloud->GetDueDate() < Pi::game->GetTime()) {
			// they emerged from hyperspace some time ago
			Ship *ship = cloud->EvictShip();

			ship->SetFrame(m_space->GetRootFrame());
			ship->SetVelocity(vector3d(0, 0, -100.0));
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
					pos =
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
						ship->SetPosition(MathUtil::RandomPointOnSphere(1000.0) * 1000.0); // somewhere 1000km out
					}

					else {
						if (sbody->GetType() == SystemBody::TYPE_STARPORT_SURFACE) {
							sbody = sbody->GetParent();
							SystemPath path = m_space->GetStarSystem()->GetPathOf(sbody);
							target_body = m_space->FindBodyForPath(&path);
						}

						double sdist = sbody->GetRadius() * 2.0;

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

	m_space->GetBackground()->SetDrawFlags(Background::Container::DRAW_SKYBOX | Background::Container::DRAW_STARS);

	// HACK: we call RebuildObjectTrees to make the internal state of CollisionSpace valid
	// This is absolutely not our job and CollisionSpace should be redesigned to fix this.
	Frame::GetFrame(m_player->GetFrame())->GetCollisionSpace()->RebuildObjectTrees();
}

const float Game::s_timeAccelRates[] = {
	0.0f,	  // paused
	1.0f,	  // 1x
	10.0f,	  // 10x
	100.0f,	  // 100x
	1000.0f,  // 1000x
	10000.0f, // 10000x
	100000.0f // hyperspace
};

const float Game::s_timeInvAccelRates[] = {
	0.0f,	 // paused
	1.0f,	 // 1x
	0.1f,	 // 10x
	0.01f,	 // 100x
	0.001f,	 // 1000x
	0.0001f, // 10000x
	0.00001f // hyperspace
};

void Game::SetTimeAccel(TimeAccel t)
{
	// don't want player to spin like mad when hitting time accel
	if ((t != m_timeAccel) && (t > TIMEACCEL_1X) &&
		m_player->GetPlayerController()->GetRotationDamping()) {
		m_player->SetAngVelocity(vector3d(0, 0, 0));
		m_player->SetTorque(vector3d(0, 0, 0));
		m_player->SetAngThrusterState(vector3d(0.0));
	}

	// Give all ships a half-step acceleration to stop autopilot overshoot
	if (t < m_timeAccel)
		for (Body *b : m_space->GetBodies())
			if (b->IsType(ObjectType::SHIP))
				(static_cast<Ship *>(b))->TimeAccelAdjust(0.5f * GetTimeStep());

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
	switch (m_requestedTimeAccel) {
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
	switch (m_requestedTimeAccel) {
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

#if WITH_OBJECTVIEWER
ObjectViewerView *Game::GetObjectViewerView() const
{
	return m_gameViews->m_objectViewerView;
}
#endif

Game::Views::Views() :
	m_sectorView(nullptr),
	m_systemView(nullptr),
	m_worldView(nullptr),
	m_deathView(nullptr),
	m_spaceStationView(nullptr),
	m_infoView(nullptr)
{}

void Game::Views::SetRenderer(Graphics::Renderer *r)
{
	// view manager will handle setting this probably
	m_infoView->SetRenderer(r);
	m_sectorView->SetRenderer(r);
	m_systemView->SetRenderer(r);
	m_worldView->SetRenderer(r);
	m_deathView->SetRenderer(r);

#if WITH_OBJECTVIEWER
	m_objectViewerView->SetRenderer(r);
#endif
}

void Game::Views::Init(Game *game)
{
	m_sectorView = new SectorView(game);
	m_worldView = new WorldView(game);
	m_systemView = new SystemView(game);
	m_spaceStationView = new PiGuiView("StationView");
	m_infoView = new PiGuiView("InfoView");
	m_deathView = new DeathView(game);

#if WITH_OBJECTVIEWER
	m_objectViewerView = new ObjectViewerView();
#endif

	SetRenderer(Pi::renderer);
}

void Game::Views::LoadFromJson(const Json &jsonObj, Game *game)
{
	m_sectorView = new SectorView(jsonObj, game);
	m_worldView = new WorldView(jsonObj, game);

	m_systemView = new SystemView(game);
	m_spaceStationView = new PiGuiView("StationView");
	m_infoView = new PiGuiView("InfoView");
	m_deathView = new DeathView(game);

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

	delete m_deathView;
	delete m_infoView;
	delete m_spaceStationView;
	delete m_systemView;
	delete m_worldView;
	delete m_sectorView;
}

// XXX this should be in some kind of central UI management class that
// creates a set of UI views held by the game. right now though the views
// are rather fundamentally tied to their global points and assume they
// can all talk to each other. given the difficulty of disentangling all
// that and the impending move to Rocket, its better right now to just
// manage creation and destruction here to get the timing and order right
void Game::CreateViews()
{
	PROFILE_SCOPED()
	Pi::SetView(nullptr);

	// XXX views expect Pi::game and Pi::player to exist
	Pi::game = this;
	Pi::player = m_player.get();

	m_gameViews.reset(new Views);
	m_gameViews->Init(this);

	log = new GameLog();
}

// XXX mostly a copy of CreateViews
void Game::LoadViewsFromJson(const Json &jsonObj)
{
	Pi::SetView(nullptr);

	// XXX views expect Pi::game and Pi::player to exist
	Pi::game = this;
	Pi::player = m_player.get();

	m_gameViews.reset(new Views);
	m_gameViews->LoadFromJson(jsonObj, this);

	log = new GameLog();
}

void Game::DestroyViews()
{
	Pi::SetView(nullptr);

	m_gameViews.reset();

	delete log;
	log = 0;
}

void Game::EmitPauseState(bool paused)
{
	if (paused) {
		// Notify UI that time is paused.
		LuaEvent::Queue("onGamePaused");
	} else {
		// Notify the UI that time is running again.
		LuaEvent::Queue("onGameResumed");
	}
	LuaEvent::Emit();
}

Json Game::LoadGameToJson(const std::string &filename)
{
	Json rootNode = JsonUtils::LoadJsonSaveFile(FileSystem::JoinPathBelow(Pi::SAVE_DIR_NAME, filename), FileSystem::userFiles);
	if (!rootNode.is_object()) {
		Output("Loading saved game '%s' failed.\n", filename.c_str());
		throw SavedGameCorruptException();
	}
	if (!rootNode["version"].is_number_integer() || rootNode["version"].get<int>() != s_saveVersion) {
		Output("Loading saved game '%s' failed: wrong save file version.\n", filename.c_str());
		throw SavedGameCorruptException();
	}
	return rootNode;
}

Game *Game::LoadGame(const std::string &filename)
{
	Output("Game::LoadGame('%s')\n", filename.c_str());

	Json rootNode = LoadGameToJson(filename);

	try {
		return new Game(rootNode);
	} catch (const Json::type_error &) {
		throw SavedGameCorruptException();
	} catch (const Json::out_of_range &) {
		throw SavedGameCorruptException();
	}
}

bool Game::CanLoadGame(const std::string &filename)
{
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

	Json rootNode;
	game->ToJson(rootNode); // Encode the game data as JSON and give to the root value.
	std::vector<uint8_t> jsonData;
	{
		PROFILE_SCOPED_DESC("json.to_cbor");
		jsonData = Json::to_cbor(rootNode); // Convert the JSON data to CBOR.
	}

	FileSystem::userFiles.MakeDirectory(Pi::SAVE_DIR_NAME);
	FILE *f = FileSystem::userFiles.OpenWriteStream(FileSystem::JoinPathBelow(Pi::SAVE_DIR_NAME, filename));
	if (!f) throw CouldNotOpenFileException();

	try {
		// Compress the CBOR data.
		const std::string comressed_data = gzip::CompressGZip(
			std::string(reinterpret_cast<const char *>(jsonData.data()), jsonData.size()),
			filename + ".json");
		size_t nwritten = fwrite(comressed_data.data(), comressed_data.size(), 1, f);
		fclose(f);
		if (nwritten != 1) throw CouldNotWriteToFileException();
	} catch (gzip::CompressionFailedException) {
		fclose(f);
		throw CouldNotWriteToFileException();
	}

	Pi::GetApp()->RequestProfileFrame("SaveGame");
}
