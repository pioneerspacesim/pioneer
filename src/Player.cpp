#include "Pi.h"
#include "Player.h"
#include "Frame.h"
#include "WorldView.h"
#include "SpaceStationView.h"
#include "Serializer.h"
#include "Sound.h"
#include "ShipCpanel.h"
#include "KeyBindings.h"

Player::Player(ShipType::Type shipType): Ship(shipType)
{
	m_mouseActive = false;
	m_flightControlState = CONTROL_MANUAL;
	m_killCount = 0;
	m_knownKillCount = 0;
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
	wr.Int32(static_cast<int>(m_flightControlState));
	wr.Double(m_setSpeed);
	wr.Int32(m_killCount);
	wr.Int32(m_knownKillCount);
}

void Player::Load(Serializer::Reader &rd)
{
	Pi::player = this;
	Ship::Load(rd);
	m_flightControlState = static_cast<FlightControlState>(rd.Int32());
	m_setSpeed = rd.Double();
	m_killCount = rd.Int32();
	m_knownKillCount = rd.Int32();
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
			Pi::cpan->MsgLog()->ImportantMessage("Pioneering Pilot's Guild", "Well done commander! Your combat rating has improved!");
		}
		m_knownKillCount = m_killCount;

		Pi::SetView(Pi::spaceStationView);
	}
}

void Player::TimeStepUpdate(const float timeStep)
{
	Ship::TimeStepUpdate(timeStep);
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
	float v_env = (Pi::worldView->GetCamType() == WorldView::CAM_EXTERNAL ? 1.0f : 0.5f);
	static Sound::Event sndev;
	float volBoth = 0.0f;
	volBoth += 0.5f*fabs(GetThrusterState().y);
	volBoth += 0.5f*fabs(GetThrusterState().z);
	
	float targetVol[2] = { volBoth, volBoth };
	if (GetThrusterState().x > 0.0)
		targetVol[0] += 0.5f*(float)GetThrusterState().x;
	else targetVol[1] += -0.5f*(float)GetThrusterState().x;

	targetVol[0] = v_env * Clamp(targetVol[0], 0.0f, 1.0f);
	targetVol[1] = v_env * Clamp(targetVol[1], 0.0f, 1.0f);
	float dv_dt[2] = { 4.0f, 4.0f };
	if (!sndev.VolumeAnimate(targetVol, dv_dt)) {
		sndev.Play("Thruster_large", 0.0f, 0.0f, Sound::OP_REPEAT);
		sndev.VolumeAnimate(targetVol, dv_dt);
	}
	float angthrust = 0.1f * v_env * (float)Pi::player->GetAngThrusterState().Length();

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
	double time_accel = Pi::GetTimeAccel();
	double invTimeAccel = 1.0 / time_accel;
	static bool stickySpeedKey = false;

	if ((time_accel == 0) || GetDockedWith() || Pi::player->IsDead() ||
	    (GetFlightState() != FLYING)) {
		return;
	}

	// if flying 
	{
		ClearThrusterState();
		
		vector3d wantAngVel(0.0);

		// have to use this function. SDL mouse position event is bugged in windows
		int mouseMotion[2];
		SDL_GetRelativeMouseState (mouseMotion+0, mouseMotion+1);	// call to flush
		if (Pi::MouseButtonState(3))
		{
			matrix4x4d rot; GetRotMatrix(rot);
			if (!m_mouseActive) {
				m_mouseDir = vector3d(-rot[8],-rot[9],-rot[10]);	// in world space
				m_mouseX = m_mouseY = 0;
				m_mouseActive = true;
			}
			vector3d objDir = m_mouseDir * rot;

			m_mouseX += mouseMotion[0] * 0.002;
			double modx = clipmouse(objDir.x, m_mouseX);			
			m_mouseX -= modx;

			if (!Pi::IsMouseYInvert()) {
				m_mouseY += mouseMotion[1] * 0.002;		// factor pixels => radians
			} else {
				m_mouseY =+ mouseMotion[1] * 0.002 * -1;
			}
			double mody = clipmouse(objDir.y, m_mouseY);
			m_mouseY -= mody;

			if(modx != 0.0 || mody != 0.0) {
				matrix4x4d mrot = matrix4x4d::RotateYMatrix(modx); mrot.RotateX(mody);
				m_mouseDir = (rot * (mrot * objDir)).Normalized();
			}
		}
		else m_mouseActive = false;
		
	
		if (m_flightControlState == CONTROL_FIXSPEED) {
			double oldSpeed = m_setSpeed;
			if (stickySpeedKey) {
				if (!(KeyBindings::increaseSpeed.IsActive() || KeyBindings::decreaseSpeed.IsActive())) {
					stickySpeedKey = false;
				}
			}
			
			if (!stickySpeedKey) {
				if (KeyBindings::increaseSpeed.IsActive()) m_setSpeed += std::max(m_setSpeed*0.05, 1.0);
				if (KeyBindings::decreaseSpeed.IsActive()) m_setSpeed -= std::max(m_setSpeed*0.05, 1.0);
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
		
		SetGunState(0,0);
		SetGunState(1,0);
		if (KeyBindings::fireLaser.IsActive() || (Pi::MouseButtonState(1) && Pi::MouseButtonState(3))) {
				SetGunState(Pi::worldView->GetActiveWeapon(), 1);
		}

		if (KeyBindings::yawLeft.IsActive()) wantAngVel.y += 1.0;
		if (KeyBindings::yawRight.IsActive()) wantAngVel.y += -1.0;
		if (KeyBindings::pitchDown.IsActive()) wantAngVel.x += -1.0;
		if (KeyBindings::pitchUp.IsActive()) wantAngVel.x += 1.0;
		if (KeyBindings::rollLeft.IsActive()) wantAngVel.z += 1.0;
		if (KeyBindings::rollRight.IsActive()) wantAngVel.z -= 1.0;

		wantAngVel.x += 2 * KeyBindings::pitchAxis.GetValue();
		wantAngVel.y += 2 * KeyBindings::yawAxis.GetValue();
		wantAngVel.z += 2 * KeyBindings::rollAxis.GetValue();

		for (int axis=0; axis<3; axis++)
			wantAngVel[axis] = Clamp(wantAngVel[axis], -invTimeAccel, invTimeAccel);

//		matrix4x4d rot; GetRotMatrix(rot);
		const double angThrustSoftness = KeyBindings::fastRotate.IsActive() ? 10.0 : 50.0;
		
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
				Pi::cpan->MsgLog()->Message("", "Alert cancelled.");
			break;

		case ALERT_SHIP_NEARBY:
			if (prev == ALERT_NONE)
				Pi::cpan->MsgLog()->ImportantMessage("", "Ship detected nearby.");
			else
				Pi::cpan->MsgLog()->ImportantMessage("", "No fire detected for 60 seconds, downgrading alert status.");
			Sound::PlaySfx("OK");
			break;

		case ALERT_SHIP_FIRING:
			Pi::cpan->MsgLog()->ImportantMessage("", "Laser fire detected.");
			Sound::PlaySfx("warning");
			break;
	}

	Pi::cpan->SetAlertState(as);

	Ship::SetAlertState(as);
}
