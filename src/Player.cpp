#include "Pi.h"
#include "Player.h"
#include "Frame.h"
#include "WorldView.h"
#include "SpaceStation.h"
#include "SpaceStationView.h"
#include "Serializer.h"
#include "Sound.h"
#include "ShipCpanel.h"
#include "KeyBindings.h"
#include "Lang.h"
#include "SectorView.h"

Player::Player(ShipType::Type shipType): Ship(shipType)
{
	m_mouseActive = false;
	m_flightControlState = CONTROL_MANUAL;
	m_killCount = 0;
	m_knownKillCount = 0;
	m_navTarget = 0;
	m_combatTarget = 0;
	UpdateMass();

	m_accumTorque = vector3d(0,0,0);
}

Player::~Player()
{
	assert(this == Pi::player);
	Pi::player = 0;
}

void Player::Save(Serializer::Writer &wr)
{
	Ship::Save(wr);
	MarketAgent::Save(wr);
	wr.Int32(static_cast<int>(m_flightControlState));
	wr.Double(m_setSpeed);
	wr.Int32(m_killCount);
	wr.Int32(m_knownKillCount);
	wr.Int32(Serializer::LookupBody(m_combatTarget));
	wr.Int32(Serializer::LookupBody(m_navTarget));
}

void Player::Load(Serializer::Reader &rd)
{
	Pi::player = this;
	Ship::Load(rd);
	MarketAgent::Load(rd);
	m_flightControlState = static_cast<FlightControlState>(rd.Int32());
	m_setSpeed = rd.Double();
	m_killCount = rd.Int32();
	m_knownKillCount = rd.Int32();
	m_combatTargetIndex = rd.Int32();
	m_navTargetIndex = rd.Int32();
}

void Player::PostLoadFixup()
{
	Ship::PostLoadFixup();
	m_combatTarget = Serializer::LookupBody(m_combatTargetIndex);
	m_navTarget = Serializer::LookupBody(m_navTargetIndex);
}

void Player::OnHaveKilled(Body *guyWeKilled)
{
	if (guyWeKilled->IsType(Object::SHIP)) {
		printf("Well done. you killed some poor fucker\n");
		m_killCount++;
	}
}

bool Player::OnDamage(Object *attacker, float kgDamage)
{
	bool r = Ship::OnDamage(attacker, kgDamage);
	if (!IsDead() && (GetPercentHull() < 25.0f)) {
		Sound::BodyMakeNoise(this, "warning", .5f);
	}
	return r;
}

void Player::SetFlightControlState(enum FlightControlState s)
{
	m_flightControlState = s;
	if (m_flightControlState == CONTROL_AUTOPILOT) {
		AIClearInstructions();
	} else if (m_flightControlState == CONTROL_FIXSPEED) {
		AIClearInstructions();
		m_setSpeed = GetVelocity().Length();
	} else {
		AIClearInstructions();
	}
	Pi::onPlayerChangeFlightControlState.emit();
}

void Player::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	if (!IsDead()) Ship::Render(viewCoords, viewTransform);
}

void Player::SetDockedWith(SpaceStation *s, int port)
{
	Ship::SetDockedWith(s, port);
	if (s) {
		if (Pi::CombatRating(m_killCount) > Pi::CombatRating(m_knownKillCount)) {
			Pi::cpan->MsgLog()->ImportantMessage(Lang::PIONEERING_PILOTS_GUILD, Lang::RIGHT_ON_COMMANDER);
		}
		m_knownKillCount = m_killCount;

		Pi::SetView(Pi::spaceStationView);
	}
}

void Player::TimeStepUpdate(const float timeStep)
{
	Ship::TimeStepUpdate(timeStep);

	if (GetFlightState() == Ship::HYPERSPACE)
	{
		Pi::RequestTimeAccel(6);

		if (Pi::GetGameTime() > m_hyperspaceEndTime)
			LeaveHyperspace();
		else
			m_hyperspaceProgress += timeStep;
	}
}

void Player::StaticUpdate(const float timeStep)
{
	vector3d v;
	matrix4x4d m;

	Ship::StaticUpdate(timeStep);		// also calls autopilot AI

	if (GetFlightState() == Ship::FLYING) {
		switch (m_flightControlState) {
		case CONTROL_FIXSPEED:
			if (Pi::GetView() == Pi::worldView) PollControls(timeStep);
			if (IsAnyThrusterKeyDown()) break;
			GetRotMatrix(m);
			v = m * vector3d(0, 0, -m_setSpeed);
			AIMatchVel(v);
			break;
		case CONTROL_MANUAL:
			if (Pi::GetView() == Pi::worldView) PollControls(timeStep);
			break;
		case CONTROL_AUTOPILOT:
			if (AIIsActive()) break;
			Pi::RequestTimeAccel(1);
//			AIMatchVel(vector3d(0.0));			// just in case autopilot doesn't...
						// actually this breaks last timestep slightly in non-relative target cases
			AIMatchAngVelObjSpace(vector3d(0.0));
			if (GetFrame()->IsRotatingFrame()) SetFlightControlState(CONTROL_FIXSPEED);
			else SetFlightControlState(CONTROL_MANUAL);
			m_setSpeed = 0.0;
			break;
		}
	}
	else SetFlightControlState(CONTROL_MANUAL);
	
	/* This wank probably shouldn't be in Player... */
	/* Ship engine noise. less loud inside */
	float v_env = (Pi::worldView->GetCamType() == WorldView::CAM_EXTERNAL ? 1.0f : 0.5f) * Sound::GetSfxVolume();
	static Sound::Event sndev;
	float volBoth = 0.0f;
	volBoth += 0.5f*fabs(GetThrusterState().y);
	volBoth += 0.5f*fabs(GetThrusterState().z);
	
	float targetVol[2] = { volBoth, volBoth };
	if (GetThrusterState().x > 0.0)
		targetVol[0] += 0.5f*float(GetThrusterState().x);
	else targetVol[1] += -0.5f*float(GetThrusterState().x);

	targetVol[0] = v_env * Clamp(targetVol[0], 0.0f, 1.0f);
	targetVol[1] = v_env * Clamp(targetVol[1], 0.0f, 1.0f);
	float dv_dt[2] = { 4.0f, 4.0f };
	if (!sndev.VolumeAnimate(targetVol, dv_dt)) {
		sndev.Play("Thruster_large", 0.0f, 0.0f, Sound::OP_REPEAT);
		sndev.VolumeAnimate(targetVol, dv_dt);
	}
	float angthrust = 0.1f * v_env * float(Pi::player->GetAngThrusterState().Length());

	static Sound::Event angThrustSnd;
	if (!angThrustSnd.VolumeAnimate(angthrust, angthrust, 5.0f, 5.0f)) {
		angThrustSnd.Play("Thruster_Small", 0.0f, 0.0f, Sound::OP_REPEAT);
		angThrustSnd.VolumeAnimate(angthrust, angthrust, 5.0f, 5.0f);
	}
}

// mouse wraparound control function
static double clipmouse(double cur, double inp)
{
	if (cur*cur > 0.7 && cur*inp > 0) return 0.0;
	if (inp > 0.2) return 0.2;
	if (inp < -0.2) return -0.2;
	return inp;
}

void Player::PollControls(const float timeStep)
{
	static bool stickySpeedKey = false;

	if (Pi::IsTimeAccelPause() || Pi::player->IsDead() || GetFlightState() != FLYING)
		return;

	// if flying 
	{
		ClearThrusterState();
		SetGunState(0,0);
		SetGunState(1,0);

		vector3d wantAngVel(0.0);
		double angThrustSoftness = 50.0;

		// have to use this function. SDL mouse position event is bugged in windows
		int mouseMotion[2];
		SDL_GetRelativeMouseState (mouseMotion+0, mouseMotion+1);	// call to flush
		if (Pi::MouseButtonState(SDL_BUTTON_RIGHT))
		{
			matrix4x4d rot; GetRotMatrix(rot);
			if (!m_mouseActive) {
				m_mouseDir = vector3d(-rot[8],-rot[9],-rot[10]);	// in world space
				m_mouseX = m_mouseY = 0;
				m_mouseActive = true;
			}
			vector3d objDir = m_mouseDir * rot;

			const double radiansPerPixel = 0.002;

			m_mouseX += mouseMotion[0] * radiansPerPixel;
			double modx = clipmouse(objDir.x, m_mouseX);			
			m_mouseX -= modx;

			m_mouseY += mouseMotion[1] * radiansPerPixel * (Pi::IsMouseYInvert() ? -1 : 1);
			double mody = clipmouse(objDir.y, m_mouseY);
			m_mouseY -= mody;

			if(!float_is_zero_general(modx) || !float_is_zero_general(mody)) {
				matrix4x4d mrot = matrix4x4d::RotateYMatrix(modx); mrot.RotateX(mody);
				m_mouseDir = (rot * (mrot * objDir)).Normalized();
			}
		}
		else m_mouseActive = false;

		// disable all keyboard controls while the console is active
		if (!Pi::IsConsoleActive()) {
			if (m_flightControlState == CONTROL_FIXSPEED) {
				double oldSpeed = m_setSpeed;
				if (stickySpeedKey) {
					if (!(KeyBindings::increaseSpeed.IsActive() || KeyBindings::decreaseSpeed.IsActive())) {
						stickySpeedKey = false;
					}
				}
				
				if (!stickySpeedKey) {
					if (KeyBindings::increaseSpeed.IsActive())
						m_setSpeed += std::max(fabs(m_setSpeed)*0.05, 1.0);
					if (KeyBindings::decreaseSpeed.IsActive())
						m_setSpeed -= std::max(fabs(m_setSpeed)*0.05, 1.0);
					if ( ((oldSpeed < 0.0) && (m_setSpeed >= 0.0)) ||
						 ((oldSpeed > 0.0) && (m_setSpeed <= 0.0)) ) {
						// flipped from going forward to backwards. make the speed 'stick' at zero
						// until the player lets go of the key and presses it again
						stickySpeedKey = true;
						m_setSpeed = 0;
					}
				}
			}

			if (KeyBindings::thrustForward.IsActive()) SetThrusterState(2, -1.0);
			if (KeyBindings::thrustBackwards.IsActive()) SetThrusterState(2, 1.0);
			if (KeyBindings::thrustUp.IsActive()) SetThrusterState(1, 1.0);
			if (KeyBindings::thrustDown.IsActive()) SetThrusterState(1, -1.0);
			if (KeyBindings::thrustLeft.IsActive()) SetThrusterState(0, -1.0);
			if (KeyBindings::thrustRight.IsActive()) SetThrusterState(0, 1.0);

			if (KeyBindings::fireLaser.IsActive() || (Pi::MouseButtonState(SDL_BUTTON_LEFT) && Pi::MouseButtonState(SDL_BUTTON_RIGHT))) {
					SetGunState(Pi::worldView->GetActiveWeapon(), 1);
			}

			if (KeyBindings::yawLeft.IsActive()) wantAngVel.y += 1.0;
			if (KeyBindings::yawRight.IsActive()) wantAngVel.y += -1.0;
			if (KeyBindings::pitchDown.IsActive()) wantAngVel.x += -1.0;
			if (KeyBindings::pitchUp.IsActive()) wantAngVel.x += 1.0;
			if (KeyBindings::rollLeft.IsActive()) wantAngVel.z += 1.0;
			if (KeyBindings::rollRight.IsActive()) wantAngVel.z -= 1.0;

			if (KeyBindings::fastRotate.IsActive())
				angThrustSoftness = 10.0;
		}

		wantAngVel.x += 2 * KeyBindings::pitchAxis.GetValue();
		wantAngVel.y += 2 * KeyBindings::yawAxis.GetValue();
		wantAngVel.z += 2 * KeyBindings::rollAxis.GetValue();

		double invTimeAccel = 1.0 / Pi::GetTimeAccel();
		for (int axis=0; axis<3; axis++)
			wantAngVel[axis] = Clamp(wantAngVel[axis], -invTimeAccel, invTimeAccel);
		
		if (m_mouseActive) AIFaceDirection(m_mouseDir);
		else AIModelCoordsMatchAngVel(wantAngVel, angThrustSoftness);
	}
}

bool Player::SetWheelState(bool down)
{
	static Sound::Event sndev;
	bool did = Ship::SetWheelState(down);
	if (did) {
		sndev.Play(down ? "UC_out" : "UC_in", 1.0f, 1.0f, 0);
	}
	return did;
}

bool Player::FireMissile(int idx, Ship *target)
{
	if (!Ship::FireMissile(idx, target))
		return false;
	
	Sound::PlaySfx("Missile_launch", 1.0f, 1.0f, 0);
	return true;
}

void Player::SetAlertState(Ship::AlertState as)
{
	Ship::AlertState prev = GetAlertState();

	switch (as) {
		case ALERT_NONE:
			if (prev != ALERT_NONE)
				Pi::cpan->MsgLog()->Message("", Lang::ALERT_CANCELLED);
			break;

		case ALERT_SHIP_NEARBY:
			if (prev == ALERT_NONE)
				Pi::cpan->MsgLog()->ImportantMessage("", Lang::SHIP_DETECTED_NEARBY);
			else
				Pi::cpan->MsgLog()->ImportantMessage("", Lang::DOWNGRADING_ALERT_STATUS);
			Sound::PlaySfx("OK");
			break;

		case ALERT_SHIP_FIRING:
			Pi::cpan->MsgLog()->ImportantMessage("", Lang::LASER_FIRE_DETECTED);
			Sound::PlaySfx("warning", 0.2f, 0.2f, 0);
			break;
	}

	Pi::cpan->SetAlertState(as);

	Ship::SetAlertState(as);
}

bool Player::IsAnyThrusterKeyDown()
{
	return !Pi::IsConsoleActive() && (
		KeyBindings::thrustForward.IsActive()	||
		KeyBindings::thrustBackwards.IsActive()	||
		KeyBindings::thrustUp.IsActive()		||
		KeyBindings::thrustDown.IsActive()		||
		KeyBindings::thrustLeft.IsActive()		||
		KeyBindings::thrustRight.IsActive()
	);
}

void Player::SetNavTarget(Body* const target)
{
	m_navTarget = target;
	Pi::onPlayerChangeTarget.emit();
	Sound::PlaySfx("OK");
}

void Player::SetCombatTarget(Body* const target)
{
	m_combatTarget = target;
	Pi::onPlayerChangeTarget.emit();
	Sound::PlaySfx("OK");
}

void Player::NotifyDeleted(const Body* const deletedBody)
{
	if(GetNavTarget() == deletedBody)
		SetNavTarget(0);
	if(GetCombatTarget() == deletedBody)
		SetCombatTarget(0);
	Ship::NotifyDeleted(deletedBody);
}

/* MarketAgent shite */
void Player::Bought(Equip::Type t)
{
	m_equipment.Add(t);
	UpdateMass();
}

void Player::Sold(Equip::Type t)
{
	m_equipment.Remove(t, 1);
	UpdateMass();
}

bool Player::CanBuy(Equip::Type t, bool verbose) const
{
	Equip::Slot slot = Equip::types[int(t)].slot;
	bool freespace = (m_equipment.FreeSpace(slot)!=0);
	bool freecapacity = (m_stats.free_capacity >= Equip::types[int(t)].mass);
	if (verbose) {
		if (!freespace) {
			Pi::Message(Lang::NO_FREE_SPACE_FOR_ITEM);
		}
		else if (!freecapacity) {
			Pi::Message(Lang::SHIP_IS_FULLY_LADEN);
		}
	}
	return (freespace && freecapacity);
}

bool Player::CanSell(Equip::Type t, bool verbose) const
{
	Equip::Slot slot = Equip::types[int(t)].slot;
	bool cansell = (m_equipment.Count(slot, t) > 0);
	if (verbose) {
		if (!cansell) {
			Pi::Message(stringf(Lang::YOU_DO_NOT_HAVE_ANY_X, formatarg("item", Equip::types[int(t)].name)));
		}
	}
	return cansell;
}

Sint64 Player::GetPrice(Equip::Type t) const
{
	if (Ship::GetDockedWith()) {
		return Ship::GetDockedWith()->GetPrice(t);
	} else {
		assert(0);
		return 0;
	}
}

void Player::EnterHyperspace()
{
	m_hyperspaceSource = Pi::spaceManager->GetCurrentSpace()->GetStarSystem()->GetPath();

	const SystemPath dest = GetHyperspaceDest();

	int fuel;
	double duration;
	Ship::HyperjumpStatus status;
	if (!CanHyperspaceTo(&dest, fuel, duration, &status))
		// XXX something has changed (fuel loss, mass change, whatever).
		// could report it to the player but better would be to cancel the
		// countdown before this is reached. either way do something
		return;

	UseHyperspaceFuel(&dest);

	Pi::luaOnLeaveSystem->Queue(this);

	if (Pi::player->GetFlightControlState() == Player::CONTROL_AUTOPILOT)
		Pi::player->SetFlightControlState(Player::CONTROL_MANUAL);

	// find all the departure clouds, convert them to arrival clouds and store
	// them for the next system
	m_hyperspaceClouds.clear();
	for (Space::BodyIterator i = Pi::spaceManager->GetCurrentSpace()->GetActiveBodies().begin(); i != Pi::spaceManager->GetCurrentSpace()->GetActiveBodies().end(); ++i) {

		if (!(*i)->IsType(Object::HYPERSPACECLOUD)) continue;

		// only want departure clouds with ships in them
		HyperspaceCloud *cloud = static_cast<HyperspaceCloud*>(*i);
		if (cloud->IsArrival() || cloud->GetShip() == 0)
			continue;

		// make sure they're going to the same place as us
		if (!dest.IsSameSystem(cloud->GetShip()->GetHyperspaceDest()))
			continue;

		// remove it from space
		Pi::spaceManager->GetCurrentSpace()->RemoveBody(cloud);

		// player and the clouds are coming to the next system, but we don't
		// want the player to have any memory of what they were (we're just
		// reusing them for convenience). tell the player it was deleted so it
		// can clean up
		Pi::player->NotifyDeleted(cloud);

		// turn the cloud arround
		cloud->GetShip()->SetHyperspaceDest(Pi::spaceManager->GetCurrentSpace()->GetStarSystem()->GetPath());
		cloud->SetIsArrival(true);

		// and remember it
		m_hyperspaceClouds.push_back(cloud);
	}

	printf("%lu clouds brought over\n", m_hyperspaceClouds.size());

	// remove the player from space
	Pi::spaceManager->GetCurrentSpace()->RemoveBody(this);

	// create hyperspace :)
	Space *newSpace = new Space();
	Pi::spaceManager->SetNextSpace(newSpace);

	// put the player in it
	SetFrame(newSpace->GetRootFrame());
	newSpace->AddBody(this);

	// put player at the origin. kind of unnecessary since it won't be moving
	// but at least it gives some consistency
	SetPosition(vector3d(0,0,0));
	SetVelocity(vector3d(0,0,0));
	SetRotMatrix(matrix4x4d::Identity());

	ClearThrusterState();
	SetFlightState(Ship::HYPERSPACE);

	// animation and end time counters
	m_hyperspaceProgress = 0;
	m_hyperspaceDuration = duration;
	m_hyperspaceEndTime = Pi::GetGameTime() + duration;

	printf("Started hyperspacing...\n");
}

void Player::LeaveHyperspace()
{
	// remove the player from hyperspace
	Pi::spaceManager->GetCurrentSpace()->RemoveBody(this);

	const SystemPath &dest = GetHyperspaceDest();

	// create a new space for the system
	Space *space = new Space(dest);
	Pi::spaceManager->SetNextSpace(space);

	// put the player in it
	SetFrame(space->GetRootFrame());
	space->AddBody(this);

	// place it
	SetPosition(space->GetPositionAfterHyperspace(&m_hyperspaceSource, &dest));
	SetVelocity(vector3d(0,0,-100.0));
	SetRotMatrix(matrix4x4d::Identity());

	SetFlightState(Ship::FLYING);
	SetFlightControlState(Player::CONTROL_MANUAL);

	// place the exit cloud
	HyperspaceCloud *cloud = new HyperspaceCloud(0, Pi::GetGameTime(), true);
	cloud->SetFrame(space->GetRootFrame());
	cloud->SetPosition(GetPosition());
	space->AddBody(cloud);

	for (std::list<HyperspaceCloud*>::iterator i = m_hyperspaceClouds.begin(); i != m_hyperspaceClouds.end(); ++i) {
		cloud = *i;

		cloud->SetFrame(space->GetRootFrame());
		cloud->SetPosition(space->GetPositionAfterHyperspace(&m_hyperspaceSource, &dest));

		space->AddBody(cloud);

		if (cloud->GetDueDate() < Pi::GetGameTime()) {
			// they emerged from hyperspace some time ago
			Ship *ship = cloud->EvictShip();

			ship->SetFrame(space->GetRootFrame());
			ship->SetVelocity(vector3d(0,0,-100.0));
			ship->SetRotMatrix(matrix4x4d::Identity());
			ship->Enable();
			ship->SetFlightState(Ship::FLYING);

			const SystemPath &sdest = ship->GetHyperspaceDest();
			if (sdest.bodyIndex == 0) {
				// travelling to the system as a whole, so just dump them on
				// the cloud - we can't do any better in this case
				ship->SetPosition(cloud->GetPosition());
			}

			else {
				// on their way to a body. they're already in-system so we
				// want to simulate some travel to their destination. we
				// naively assume full accel for half the distance, flip and
				// full brake for the rest.
				Body *target_body = space->FindBodyForPath(&sdest);
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
						target_body->GetPositionRelTo(space->GetRootFrame()) +
						cloud->GetPositionRelTo(target_body).Normalized() * (dist_to_target - dist);
					ship->SetPosition(pos);
				}

				else {
					// ship made it with time to spare. just put it somewhere
					// near the body. the script should be issuing a dock or
					// flyto command in onEnterSystem so it should sort it
					// itself out long before the player can get near
					
					SBody *sbody = space->GetStarSystem()->GetBodyByPath(&sdest);
					if (sbody->type == SBody::TYPE_STARPORT_ORBITAL) {
						ship->SetFrame(target_body->GetFrame());
						ship->SetPosition(space->GetRandomPosition(1000.0,1000.0)*1000.0); // somewhere 1000km out
					}

					else {
						if (sbody->type == SBody::TYPE_STARPORT_SURFACE) {
							sbody = sbody->parent;
							SystemPath path = space->GetStarSystem()->GetPathOf(sbody);
							target_body = space->FindBodyForPath(&path);
						}

						double sdist = sbody->GetRadius()*2.0;

						ship->SetFrame(target_body->GetFrame());
						ship->SetPosition(space->GetRandomPosition(sdist,sdist));
					}
				}
			}

			space->AddBody(ship);

			Pi::luaOnEnterSystem->Queue(ship);
		}
	}
	m_hyperspaceClouds.clear();

	//Pi::luaOnEnterSystem->Queue(Pi::player);

	Pi::sectorView->ResetHyperspaceTarget();

	Pi::RequestTimeAccel(1);
}
