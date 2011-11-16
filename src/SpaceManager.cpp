#include "SpaceManager.h"
#include "Space.h"
#include "Player.h"
#include "Body.h"
#include "SpaceStation.h"
#include "HyperspaceCloud.h"
#include "Pi.h"
#include "ShipCpanel.h"
#include "Sfx.h"
#include "MathUtil.h"

void SpaceManager::Serialize(Serializer::Writer &wr)
{
	Serializer::Writer section;
	m_space->Serialize(section);
	wr.WrSection("Space", section.GetData());

	wr.Int32(m_hyperspaceClouds.size());
	for (std::list<HyperspaceCloud*>::const_iterator i = m_hyperspaceClouds.begin(); i != m_hyperspaceClouds.end(); ++i)
		(*i)->Serialize(wr);

	wr.Int32(Serializer::LookupBody(m_player));
	wr.Int32(Uint32(m_state));
	wr.Bool(m_wantHyperspace);
	wr.Double(m_hyperspaceProgress);
	wr.Double(m_hyperspaceDuration);
	wr.Double(m_hyperspaceEndTime);
}

void SpaceManager::CreateSpaceForDockedStart(const SystemPath &path)
{
	assert(m_state == STATE_NONE);
	assert(!m_space);

	m_space.Reset(new Space(path));
	m_space->TimeStep(0);
	
	SpaceStation *station = 0;
	Uint32 idx = path.bodyIndex;
	for (Space::BodyIterator i = m_space->IteratorBegin(); i != m_space->IteratorEnd(); ++i)
		if (--idx == 0) {
			assert((*i)->IsType(Object::SPACESTATION));
			station = static_cast<SpaceStation*>(*i);
			break;
		}
	assert(station);

	m_space->AddBody(m_player);

	m_player->Enable();
	m_player->SetFrame(station->GetFrame());
	m_player->SetDockedWith(station, 0);

	// XXX stupid, should probably be done by SetDockedWith
	station->CreateBB();

    m_state = STATE_NORMAL;
}

void SpaceManager::CreateSpaceForFreeStart(const SystemPath &path, const vector3d &pos)
{
	assert(m_state == STATE_NONE);
	assert(!m_space);

	m_space.Reset(new Space(path));
	m_space->TimeStep(0);
	
	Body *b = 0;
	Uint32 idx = path.bodyIndex;
	for (Space::BodyIterator i = m_space->IteratorBegin(); i != m_space->IteratorEnd(); ++i)
		if (--idx == 0) {
			b = *i;
			break;
		}
	assert(b);

	m_space->AddBody(m_player);

	m_player->Enable();
	m_player->SetFrame(b->GetFrame());

	m_player->SetPosition(pos);
	m_player->SetVelocity(vector3d(0,0,0));

    m_state = STATE_NORMAL;
}

void SpaceManager::TimeStep(float step)
{
	assert(m_state != STATE_NONE);
	assert(m_space);

	m_space->TimeStep(step);

	// XXX ui updates, not sure if they belong here
	Pi::cpan->TimeStepUpdate(step);
	Sfx::TimeStepAll(step, m_space->GetRootFrame());

	if (m_state == STATE_HYPERSPACE) {
		if (Pi::GetGameTime() > m_hyperspaceEndTime) {
			SwitchToNormalSpace();
			m_player->EnterSystem();
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

void SpaceManager::WantHyperspace()
{
	assert(m_state == STATE_NORMAL);
	m_wantHyperspace = true;
}

void SpaceManager::SwitchToHyperspace()
{
	// remember where we came from so we can properly place the player on exit
	m_hyperspaceSource = m_space->GetStarSystem()->GetPath();

	const SystemPath &dest = m_player->GetHyperspaceDest();

	// find all the departure clouds, convert them to arrival clouds and store
	// them for the next system
	m_hyperspaceClouds.clear();
	for (Space::BodyIterator i = m_space->IteratorBegin(); i != m_space->IteratorEnd(); ++i) {

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

	printf("%lu clouds brought over\n", m_hyperspaceClouds.size());

	// remove the player from space
	m_space->RemoveBody(m_player);

	// create hyperspace :)
	m_space.Reset(new Space());

	// put the player in it
	m_player->SetFrame(m_space->GetRootFrame());
	m_space->AddBody(m_player);

	// put player at the origin. kind of unnecessary since it won't be moving
	// but at least it gives some consistency
	m_player->SetPosition(vector3d(0,0,0));
	m_player->SetVelocity(vector3d(0,0,0));
	m_player->SetRotMatrix(matrix4x4d::Identity());

	// animation and end time counters
	m_hyperspaceProgress = 0;
	m_hyperspaceDuration = m_player->GetHyperspaceDuration();
	m_hyperspaceEndTime = Pi::GetGameTime() + m_hyperspaceDuration;

	m_state = STATE_HYPERSPACE;
	m_wantHyperspace = false;

	printf("Started hyperspacing...\n");
}

void SpaceManager::SwitchToNormalSpace()
{
	// remove the player from hyperspace
	m_space->RemoveBody(m_player);

	// create a new space for the system
	const SystemPath &dest = m_player->GetHyperspaceDest();
	m_space.Reset(new Space(dest));

	// put the player in it
	m_player->SetFrame(m_space->GetRootFrame());
	m_space->AddBody(m_player);

	// place it
	m_player->SetPosition(m_space->GetHyperspaceExitPoint(m_hyperspaceSource));
	m_player->SetVelocity(vector3d(0,0,-100.0));
	m_player->SetRotMatrix(matrix4x4d::Identity());

	// place the exit cloud
	HyperspaceCloud *cloud = new HyperspaceCloud(0, Pi::GetGameTime(), true);
	cloud->SetFrame(m_space->GetRootFrame());
	cloud->SetPosition(m_player->GetPosition());
	m_space->AddBody(cloud);

	for (std::list<HyperspaceCloud*>::iterator i = m_hyperspaceClouds.begin(); i != m_hyperspaceClouds.end(); ++i) {
		cloud = *i;

		cloud->SetFrame(m_space->GetRootFrame());
		cloud->SetPosition(m_space->GetHyperspaceExitPoint(m_hyperspaceSource));

		m_space->AddBody(cloud);

		if (cloud->GetDueDate() < Pi::GetGameTime()) {
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
				double travel_time = Pi::GetGameTime() - cloud->GetDueDate();

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
