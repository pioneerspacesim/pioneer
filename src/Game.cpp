#include "Game.h"
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
#include "GalacticView.h"
#include "SystemView.h"
#include "SystemInfoView.h"
#include "SpaceStationView.h"
#include "InfoView.h"
#include "ObjectViewerView.h"

static const int  s_saveVersion   = 45;
static const char s_saveStart[]   = "PIONEER";
static const char s_saveEnd[]     = "END";

Game::Game(const SystemPath &path) :
	m_time(0),
	m_state(STATE_NORMAL),
	m_wantHyperspace(false),
	m_timeAccel(TIMEACCEL_1X),
	m_requestedTimeAccel(TIMEACCEL_1X),
	m_forceTimeAccel(false)
{
	CreatePlayer();

	m_space.Reset(new Space(this, path));
	SpaceStation *station = static_cast<SpaceStation*>(m_space->FindBodyForPath(&path));
	assert(station);

	m_space->AddBody(m_player.Get());

	m_player->Enable();
	m_player->SetFrame(station->GetFrame());
	m_player->SetDockedWith(station, 0);

	CreateViews();
}

Game::Game(const SystemPath &path, const vector3d &pos) :
	m_time(0),
	m_state(STATE_NORMAL),
	m_wantHyperspace(false),
	m_timeAccel(TIMEACCEL_1X),
	m_requestedTimeAccel(TIMEACCEL_1X),
	m_forceTimeAccel(false)
{
	CreatePlayer();

	m_space.Reset(new Space(this, path));
	Body *b = m_space->FindBodyForPath(&path);
	assert(b);

	m_space->AddBody(m_player.Get());

	m_player->Enable();
	m_player->SetFrame(b->GetFrame());

	m_player->SetPosition(pos);
	m_player->SetVelocity(vector3d(0,0,0));

	CreateViews();
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

	m_space->RemoveBody(m_player.Get());
	m_space.Reset();
	m_player.Reset();
}

Game::Game(Serializer::Reader &rd) :
	m_timeAccel(TIMEACCEL_PAUSED),
	m_requestedTimeAccel(TIMEACCEL_PAUSED),
	m_forceTimeAccel(false)
{
	// signature check
	for (Uint32 i = 0; i < strlen(s_saveStart)+1; i++)
		if (rd.Byte() != s_saveStart[i]) throw SavedGameCorruptException();

	// version check
	rd.SetStreamVersion(rd.Int32());
	fprintf(stderr, "savefile version: %d\n", rd.StreamVersion());
	if (rd.StreamVersion() != s_saveVersion) {
		fprintf(stderr, "can't load savefile, expected version: %d\n", s_saveVersion);
		throw SavedGameCorruptException();
	}

	Serializer::Reader section;

	// space, all the bodies and things
	section = rd.RdSection("Space");
	m_space.Reset(new Space(this, section));

	
	// game state and space transition state
	section = rd.RdSection("Game");

	m_player.Reset(static_cast<Player*>(m_space->GetBodyByIndex(section.Int32())));

	// hyperspace clouds being brought over from the previous system
	Uint32 nclouds = section.Int32();
	for (Uint32 i = 0; i < nclouds; i++)
		m_hyperspaceClouds.push_back(static_cast<HyperspaceCloud*>(Body::Unserialize(section, 0)));
	
	m_time = section.Double();
	m_state = State(section.Int32());

	m_wantHyperspace = section.Bool();
	m_hyperspaceProgress = section.Double();
	m_hyperspaceDuration = section.Double();
	m_hyperspaceEndTime = section.Double();


	// system political stuff
	section = rd.RdSection("Polit");
	Polit::Unserialize(section);


	// views
	LoadViews(rd);


	// lua
	section = rd.RdSection("LuaModules");
	Pi::luaSerializer->Unserialize(section);


	// signature check
	for (Uint32 i = 0; i < strlen(s_saveEnd)+1; i++)
		if (rd.Byte() != s_saveEnd[i]) throw SavedGameCorruptException();
}

void Game::Serialize(Serializer::Writer &wr)
{
	// leading signature
	for (Uint32 i = 0; i < strlen(s_saveStart)+1; i++)
		wr.Byte(s_saveStart[i]);
	
	// version
	wr.Int32(s_saveVersion);

	Serializer::Writer section;

	// space, all the bodies and things
	m_space->Serialize(section);
	wr.WrSection("Space", section.GetData());

	
	// game state and space transition state
	section = Serializer::Writer();

	section.Int32(m_space->GetIndexForBody(m_player.Get()));

	// hyperspace clouds being brought over from the previous system
	section.Int32(m_hyperspaceClouds.size());
	for (std::list<HyperspaceCloud*>::const_iterator i = m_hyperspaceClouds.begin(); i != m_hyperspaceClouds.end(); ++i)
		(*i)->Serialize(section, m_space.Get());

	section.Double(m_time);
	section.Int32(Uint32(m_state));

	section.Bool(m_wantHyperspace);
	section.Double(m_hyperspaceProgress);
	section.Double(m_hyperspaceDuration);
	section.Double(m_hyperspaceEndTime);
	
	wr.WrSection("Game", section.GetData());


	// system political data (crime etc)
	section = Serializer::Writer();
	Polit::Serialize(section);
	wr.WrSection("Polit", section.GetData());


	// views. must be saved in init order
	section = Serializer::Writer();
	Pi::cpan->Save(section);
	wr.WrSection("ShipCpanel", section.GetData());
	
	section = Serializer::Writer();
	Pi::sectorView->Save(section);
	wr.WrSection("SectorView", section.GetData());

	section = Serializer::Writer();
	Pi::worldView->Save(section);
	wr.WrSection("WorldView", section.GetData());


	// lua
	section = Serializer::Writer();
	Pi::luaSerializer->Serialize(section);
	wr.WrSection("LuaModules", section.GetData());


	// trailing signature
	for (Uint32 i = 0; i < strlen(s_saveEnd)+1; i++)
		wr.Byte(s_saveEnd[i]);
}

void Game::TimeStep(float step)
{
	m_space->TimeStep(step);

	// XXX ui updates, not sure if they belong here
	Pi::cpan->TimeStepUpdate(step);
	Sfx::TimeStepAll(step, m_space->GetRootFrame());

	m_time += step;

	if (m_state == STATE_HYPERSPACE) {
		if (Pi::game->GetTime() > m_hyperspaceEndTime) {
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
	// don't modify the timeaccel if the game is paused
	if (m_requestedTimeAccel == Game::TIMEACCEL_PAUSED) {
		m_timeAccel = Game::TIMEACCEL_PAUSED;
		return false;
	}

	TimeAccel newTimeAccel = m_requestedTimeAccel;

	// ludicrous speed
	if (m_player->GetFlightState() == Ship::HYPERSPACE) {
		newTimeAccel = Game::TIMEACCEL_HYPERSPACE;
		RequestTimeAccel(newTimeAccel);
	}

	// force down to timeaccel 1 during the docking sequence
	else if (m_player->GetFlightState() == Ship::DOCKING) {
		newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_1X);
		RequestTimeAccel(newTimeAccel);
	}

	// normal flight
	else if (m_player->GetFlightState() == Ship::FLYING) {

		// special timeaccel lock rules while in alert
		if (m_player->GetAlertState() == Ship::ALERT_SHIP_NEARBY)
			newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_10X);
		else if (m_player->GetAlertState() == Ship::ALERT_SHIP_FIRING)
			newTimeAccel = std::min(newTimeAccel, Game::TIMEACCEL_1X);

		else if (!m_forceTimeAccel) {
			// check we aren't too near to objects for timeaccel //
			for (Space::BodyIterator i = m_space->BodiesBegin(); i != m_space->BodiesEnd(); ++i) {
				if ((*i) == m_player) continue;
				if ((*i)->IsType(Object::HYPERSPACECLOUD)) continue;
			
				vector3d toBody = m_player->GetPosition() - (*i)->GetPositionRelTo(m_player->GetFrame());
				double dist = toBody.Length();
				double rad = (*i)->GetBoundingRadius();

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
	}

	// no change
	if (newTimeAccel == m_timeAccel)
		return false;
	
	m_timeAccel = newTimeAccel;
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

void Game::SwitchToHyperspace()
{
	// remember where we came from so we can properly place the player on exit
	m_hyperspaceSource = m_space->GetStarSystem()->GetPath();

	const SystemPath &dest = m_player->GetHyperspaceDest();

	// find all the departure clouds, convert them to arrival clouds and store
	// them for the next system
	m_hyperspaceClouds.clear();
	for (Space::BodyIterator i = m_space->BodiesBegin(); i != m_space->BodiesEnd(); ++i) {

		if (!(*i)->IsType(Object::HYPERSPACECLOUD)) continue;

		// only want departure clouds with ships in them
		HyperspaceCloud *cloud = static_cast<HyperspaceCloud*>(*i);
		if (cloud->IsArrival() || cloud->GetShip() == 0)
			continue;

		// make sure they're going to the same place as us
		if (!dest.IsSameSystem(cloud->GetShip()->GetHyperspaceDest()))
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

	printf(SIZET_FMT " clouds brought over\n", m_hyperspaceClouds.size());

	// remove the player from space
	m_space->RemoveBody(m_player.Get());

	// create hyperspace :)
	m_space.Reset(new Space(this));

	// put the player in it
	m_player->SetFrame(m_space->GetRootFrame());
	m_space->AddBody(m_player.Get());

	// put player at the origin. kind of unnecessary since it won't be moving
	// but at least it gives some consistency
	m_player->SetPosition(vector3d(0,0,0));
	m_player->SetVelocity(vector3d(0,0,0));
	m_player->SetRotMatrix(matrix4x4d::Identity());

	// animation and end time counters
	m_hyperspaceProgress = 0;
	m_hyperspaceDuration = m_player->GetHyperspaceDuration();
	m_hyperspaceEndTime = Pi::game->GetTime() + m_hyperspaceDuration;

	m_state = STATE_HYPERSPACE;
	m_wantHyperspace = false;

	printf("Started hyperspacing...\n");
}

void Game::SwitchToNormalSpace()
{
	// remove the player from hyperspace
	m_space->RemoveBody(m_player.Get());

	// create a new space for the system
	const SystemPath &dest = m_player->GetHyperspaceDest();
	m_space.Reset(new Space(this, dest));

	// put the player in it
	m_player->SetFrame(m_space->GetRootFrame());
	m_space->AddBody(m_player.Get());

	// place it
	m_player->SetPosition(m_space->GetHyperspaceExitPoint(m_hyperspaceSource));
	m_player->SetVelocity(vector3d(0,0,-100.0));
	m_player->SetRotMatrix(matrix4x4d::Identity());

	// place the exit cloud
	HyperspaceCloud *cloud = new HyperspaceCloud(0, Pi::game->GetTime(), true);
	cloud->SetFrame(m_space->GetRootFrame());
	cloud->SetPosition(m_player->GetPosition());
	m_space->AddBody(cloud);

	for (std::list<HyperspaceCloud*>::iterator i = m_hyperspaceClouds.begin(); i != m_hyperspaceClouds.end(); ++i) {
		cloud = *i;

		cloud->SetFrame(m_space->GetRootFrame());
		cloud->SetPosition(m_space->GetHyperspaceExitPoint(m_hyperspaceSource));

		m_space->AddBody(cloud);

		if (cloud->GetDueDate() < Pi::game->GetTime()) {
			// they emerged from hyperspace some time ago
			Ship *ship = cloud->EvictShip();

			ship->SetFrame(m_space->GetRootFrame());
			ship->SetVelocity(vector3d(0,0,-100.0));
			ship->SetRotMatrix(matrix4x4d::Identity());
			ship->Enable();
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
				double accel = -(ship->GetShipType().linThrust[ShipType::THRUSTER_FORWARD] / ship->GetMass());
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
					
					SBody *sbody = m_space->GetStarSystem()->GetBodyByPath(&sdest);
					if (sbody->type == SBody::TYPE_STARPORT_ORBITAL) {
						ship->SetFrame(target_body->GetFrame());
						ship->SetPosition(MathUtil::RandomPointOnSphere(1000.0)*1000.0); // somewhere 1000km out
					}

					else {
						if (sbody->type == SBody::TYPE_STARPORT_SURFACE) {
							sbody = sbody->parent;
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

			Pi::luaOnEnterSystem->Queue(ship);
		}
	}
	m_hyperspaceClouds.clear();

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

void Game::SetTimeAccel(TimeAccel t)
{
	// don't want player to spin like mad when hitting time accel
	if ((t != m_timeAccel) && (t > TIMEACCEL_1X)) {
		m_player->SetAngVelocity(vector3d(0,0,0));
		m_player->SetTorque(vector3d(0,0,0));
		m_player->SetAngThrusterState(vector3d(0.0));
	}

	// Give all ships a half-step acceleration to stop autopilot overshoot
	if (t < m_timeAccel)
		for (Space::BodyIterator i = m_space->BodiesBegin(); i != m_space->BodiesEnd(); ++i)
			if ((*i)->IsType(Object::SHIP))
				(static_cast<DynamicBody*>(*i))->ApplyAccel(0.5f * GetTimeStep());

	m_timeAccel = t;

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

void Game::CreatePlayer()
{
	// XXX this should probably be in lua somewhere
	m_player.Reset(new Player("Eagle Long Range Fighter"));
	m_player->m_equipment.Set(Equip::SLOT_ENGINE, 0, Equip::DRIVE_CLASS1);
	m_player->m_equipment.Set(Equip::SLOT_LASER, 0, Equip::PULSECANNON_1MW);
	m_player->m_equipment.Add(Equip::HYDROGEN, 1);
	m_player->m_equipment.Add(Equip::ATMOSPHERIC_SHIELDING);
	m_player->m_equipment.Add(Equip::MISSILE_GUIDED);
	m_player->m_equipment.Add(Equip::MISSILE_GUIDED);
	m_player->m_equipment.Add(Equip::AUTOPILOT);
	m_player->m_equipment.Add(Equip::SCANNER);
	m_player->UpdateMass();
	m_player->SetMoney(10000);
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
	Pi::player = m_player.Get();

	Pi::cpan = new ShipCpanel();
	Pi::sectorView = new SectorView();
	Pi::worldView = new WorldView();
	Pi::galacticView = new GalacticView();
	Pi::systemView = new SystemView();
	Pi::systemInfoView = new SystemInfoView();
	Pi::spaceStationView = new SpaceStationView();
	Pi::infoView = new InfoView();

#if WITH_OBJECTVIEWER
	Pi::objectViewerView = new ObjectViewerView();
#endif
}

// XXX mostly a copy of CreateViews
void Game::LoadViews(Serializer::Reader &rd)
{
	Pi::SetView(0);

	// XXX views expect Pi::game and Pi::player to exist
	Pi::game = this;
	Pi::player = m_player.Get();

	Serializer::Reader section = rd.RdSection("ShipCpanel");
	Pi::cpan = new ShipCpanel(section);

	section = rd.RdSection("SectorView");
	Pi::sectorView = new SectorView(section);

	section = rd.RdSection("WorldView");
	Pi::worldView = new WorldView(section);

	Pi::galacticView = new GalacticView();
	Pi::systemView = new SystemView();
	Pi::systemInfoView = new SystemInfoView();
	Pi::spaceStationView = new SpaceStationView();
	Pi::infoView = new InfoView();

#if WITH_OBJECTVIEWER
	Pi::objectViewerView = new ObjectViewerView();
#endif
}

void Game::DestroyViews()
{
	Pi::SetView(0);

#if WITH_OBJECTVIEWER
	delete Pi::objectViewerView;
#endif

	delete Pi::infoView;
	delete Pi::spaceStationView;
	delete Pi::systemInfoView;
	delete Pi::systemView;
	delete Pi::galacticView;
	delete Pi::worldView;
	delete Pi::sectorView;
	delete Pi::cpan;

	Pi::objectViewerView = 0;
	Pi::infoView = 0;
	Pi::spaceStationView = 0;
	Pi::systemInfoView = 0;
	Pi::systemView = 0;
	Pi::galacticView = 0;
	Pi::worldView = 0;
	Pi::sectorView = 0;
	Pi::cpan = 0;
}
