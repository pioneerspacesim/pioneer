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
#include "Game.h"

Player::Player(ShipType::Type shipType): Ship(shipType)
{
	m_mouseActive = false;
	m_invertMouse = false;
	m_flightControlState = CONTROL_MANUAL;
	m_killCount = 0;
	m_knownKillCount = 0;
	m_setSpeedTarget = 0;
	m_navTarget = 0;
	m_combatTarget = 0;
	UpdateMass();

	float deadzone = Pi::config.Float("JoystickDeadzone");
	m_joystickDeadzone = deadzone * deadzone;

	m_accumTorque = vector3d(0,0,0);
}

void Player::Save(Serializer::Writer &wr, Space *space)
{
	Ship::Save(wr, space);
	MarketAgent::Save(wr);
	wr.Int32(static_cast<int>(m_flightControlState));
	wr.Double(m_setSpeed);
	wr.Int32(m_killCount);
	wr.Int32(m_knownKillCount);
	wr.Int32(space->GetIndexForBody(m_combatTarget));
	wr.Int32(space->GetIndexForBody(m_navTarget));
	wr.Int32(space->GetIndexForBody(m_setSpeedTarget));
}

void Player::Load(Serializer::Reader &rd, Space *space)
{
	Pi::player = this;
	Ship::Load(rd, space);
	MarketAgent::Load(rd);
	m_flightControlState = static_cast<FlightControlState>(rd.Int32());
	m_setSpeed = rd.Double();
	m_killCount = rd.Int32();
	m_knownKillCount = rd.Int32();
	m_combatTargetIndex = rd.Int32();
	m_navTargetIndex = rd.Int32();
	m_setSpeedTargetIndex = rd.Int32();
}

void Player::PostLoadFixup(Space *space)
{
	Ship::PostLoadFixup(space);
	m_combatTarget = space->GetBodyByIndex(m_combatTargetIndex);
	m_navTarget = space->GetBodyByIndex(m_navTargetIndex);
	m_setSpeedTarget = space->GetBodyByIndex(m_setSpeedTargetIndex);
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
		m_setSpeed = m_setSpeedTarget ? GetVelocityRelTo(m_setSpeedTarget).Length() : GetVelocity().Length();
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

void Player::StaticUpdate(const float timeStep)
{
	vector3d v;
	matrix4x4d m;

	if (GetFlightState() == Ship::FLYING) {
		switch (m_flightControlState) {
		case CONTROL_FIXSPEED:
			if (Pi::GetView() == Pi::worldView) PollControls(timeStep);
			if (IsAnyThrusterKeyDown()) break;
			GetRotMatrix(m);
			v = m * vector3d(0, 0, -m_setSpeed);
			if (m_setSpeedTarget) {
				v += m_setSpeedTarget->GetVelocityRelTo(GetFrame());
			}
			AIMatchVel(v);
			break;
		case CONTROL_MANUAL:
			if (Pi::GetView() == Pi::worldView) PollControls(timeStep);
			break;
		case CONTROL_AUTOPILOT:
			if (AIIsActive()) break;
			Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
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
	
	Ship::StaticUpdate(timeStep);		// also calls autopilot AI

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

	if (Pi::game->GetTimeAccel() == Game::TIMEACCEL_PAUSED || Pi::player->IsDead() || GetFlightState() != FLYING)
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

			const bool invertY = (Pi::IsMouseYInvert() ? !m_invertMouse : m_invertMouse);

			m_mouseY += mouseMotion[1] * radiansPerPixel * (invertY ? -1 : 1);
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

		vector3d changeVec;
		changeVec.x = KeyBindings::pitchAxis.GetValue();
		changeVec.y = KeyBindings::yawAxis.GetValue();
		changeVec.z = KeyBindings::rollAxis.GetValue();

		// Deadzone
		if(changeVec.LengthSqr() < m_joystickDeadzone)
			changeVec = vector3d(0.0);

		changeVec *= 2.0;
		wantAngVel += changeVec;

		double invTimeAccelRate = 1.0 / Pi::game->GetTimeAccelRate();
		for (int axis=0; axis<3; axis++)
			wantAngVel[axis] = Clamp(wantAngVel[axis], -invTimeAccelRate, invTimeAccelRate);
		
		AIModelCoordsMatchAngVel(wantAngVel, angThrustSoftness);
		if (m_mouseActive) AIFaceDirection(m_mouseDir);
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

void Player::SetNavTarget(Body* const target, bool setSpeedTo)
{
	if (setSpeedTo)
		m_setSpeedTarget = target;
	else if (m_setSpeedTarget == m_navTarget)
		m_setSpeedTarget = 0;
	m_navTarget = target;
	Pi::onPlayerChangeTarget.emit();
	Sound::PlaySfx("OK");
}

void Player::SetCombatTarget(Body* const target, bool setSpeedTo)
{
	if (setSpeedTo)
		m_setSpeedTarget = target;
	else if (m_setSpeedTarget == m_combatTarget)
		m_setSpeedTarget = 0;
	m_combatTarget = target;
	Pi::onPlayerChangeTarget.emit();
	Sound::PlaySfx("OK");
}

void Player::NotifyRemoved(const Body* const removedBody)
{
	if (GetNavTarget() == removedBody)
		SetNavTarget(0);

	else if (GetCombatTarget() == removedBody) {
		SetCombatTarget(0);

		if (!GetNavTarget() && removedBody->IsType(Object::SHIP))
			SetNavTarget(static_cast<const Ship*>(removedBody)->GetHyperspaceCloud());
	}

	Ship::NotifyRemoved(removedBody);
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

void Player::OnEnterHyperspace()
{
	SetNavTarget(0);
	SetCombatTarget(0);

	if (Pi::player->GetFlightControlState() == Player::CONTROL_AUTOPILOT)
		Pi::player->SetFlightControlState(Player::CONTROL_MANUAL);

	ClearThrusterState();

	Pi::game->WantHyperspace();
}

void Player::OnEnterSystem()
{
	SetFlightControlState(Player::CONTROL_MANUAL);

	Pi::sectorView->ResetHyperspaceTarget();
}
