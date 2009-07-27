#include "Pi.h"
#include "Player.h"
#include "Frame.h"
#include "WorldView.h"
#include "SpaceStationView.h"
#include "Serializer.h"
#include "Mission.h"

Player::Player(ShipType::Type shipType): Ship(shipType)
{
	m_mouseCMov[0] = m_mouseCMov[1] = 0;
	m_flightControlState = CONTROL_MANUAL;
	UpdateMass();
}

Player::~Player()
{
	assert(this == Pi::player);
	Pi::player = 0;
	for (std::list<Mission*>::iterator i = m_missions.begin();
			i != m_missions.end(); ++i) {
		delete (*i);
	}
}

void Player::Save()
{
	using namespace Serializer::Write;
	Ship::Save();
	wr_int(static_cast<int>(m_flightControlState));
	wr_float(m_setSpeed);
	// save missions
	wr_int(m_missions.size());
	for (std::list<Mission*>::iterator i = m_missions.begin();
			i != m_missions.end(); ++i) {
		(*i)->Save();
	}
}

void Player::Load()
{
	Pi::player = this;
	using namespace Serializer::Read;
	Ship::Load();
	m_flightControlState = static_cast<FlightControlState>(rd_int());
	m_setSpeed = rd_float();
	// load missions
	int numMissions = rd_int();
	for (int i=0; i<numMissions; i++) {
		Mission *m = Mission::Load();
		m->AttachToPlayer();
		m_missions.push_back(m);
	}
}

void Player::TakeMission(Mission *m)
{
	m_missions.push_front(m);
	Pi::onPlayerMissionListChanged.emit();
}

void Player::SetHyperspaceTarget(const SBodyPath *path)
{
	m_hyperspaceTarget = *path;
	Pi::onPlayerChangeHyperspaceTarget.emit();
}

void Player::SetFlightControlState(enum FlightControlState s)
{
	m_flightControlState = s;
	if (m_flightControlState == CONTROL_AUTOPILOT) {
		Body *target = GetNavTarget();
		AIClearInstructions();
		if (target && target->IsType(Object::SHIP)) {
			AIInstruct(Ship::DO_KILL, target);
		} else if (target) {
			AIInstruct(Ship::DO_KILL, target);
		}
	} else if (m_flightControlState == CONTROL_FIXSPEED) {
		AIClearInstructions();
		m_setSpeed = GetVelocity().Length();
	} else {
		AIClearInstructions();
	}
}

void Player::Render(const Frame *camFrame)
{
	if (Pi::worldView->GetCamType() == WorldView::CAM_EXTERNAL) {
		Ship::Render(camFrame);
	} else {
		glPushMatrix();
		// could only rotate, since transform is zero (camFrame is at player origin)
		RenderLaserfire();
		glPopMatrix();
	}
}

void Player::SetDockedWith(SpaceStation *s, int port)
{
	Ship::SetDockedWith(s, port);
	if (s) {
		Pi::SetView(Pi::spaceStationView);
	}
}

void Player::TimeStepUpdate(const float timeStep)
{
	Body *b;
	vector3d v;
	ClearThrusterState();
	if (Pi::GetView() == Pi::worldView) PollControls();

	if (GetFlightState() == Ship::FLYING) {
		switch (m_flightControlState) {
		case CONTROL_MANUAL:
			// apply rotation damping
			{const float time_accel = Pi::GetTimeAccel();
			const float invTa2 = 1.0f/(time_accel*time_accel);
		
			vector3f angThrust = GetAngThrusterState();
			vector3d damping = time_accel*CalcRotDamping();

			angThrust.x -= damping.x * invTa2;
			angThrust.y -= damping.y * invTa2;
			angThrust.z -= damping.z * invTa2;

			// dividing by time step so controls don't go totally mental when
			// used at 10x accel
			SetAngThrusterState(0, angThrust.x);
			SetAngThrusterState(1, angThrust.y);
			SetAngThrusterState(2, angThrust.z);
			}
			break;
		case CONTROL_FIXSPEED:
			b = (GetCombatTarget() ? GetCombatTarget() : GetNavTarget());
			v = vector3d(0, 0, -m_setSpeed);
			if (b) {
				matrix4x4d m;
				GetRotMatrix(m);
				v += m.InverseOf() * b->GetVelocityRelativeTo(this->GetFrame());
			}
			AIAccelToModelRelativeVelocity(v);
			break;
		case CONTROL_AUTOPILOT:
			break;
		}
	} else {
		m_flightControlState = CONTROL_MANUAL;
		AIClearInstructions();
	}
	Ship::TimeStepUpdate(timeStep);
}

#define MOUSE_CTRL_AREA		10.0f
#define MOUSE_RESTITUTION	0.75f

void Player::PollControls()
{
	int mouseMotion[2];
	float time_accel = Pi::GetTimeAccel();
	float ta2 = time_accel*time_accel;

	if ((time_accel == 0) || GetDockedWith() ||
	    (GetFlightState() != FLYING)) {
		return;
	}

	/* if flying */
	{
		ClearThrusterState();
		
		vector3f angThrust(0.0f);

		if (Pi::MouseButtonState(3)) {
			Pi::GetMouseMotion(mouseMotion);
			m_mouseCMov[0] += mouseMotion[0];
			m_mouseCMov[1] += mouseMotion[1];
			m_mouseCMov[0] = CLAMP(m_mouseCMov[0]*MOUSE_RESTITUTION, -MOUSE_CTRL_AREA, MOUSE_CTRL_AREA);
			m_mouseCMov[1] = CLAMP(m_mouseCMov[1]*MOUSE_RESTITUTION, -MOUSE_CTRL_AREA, MOUSE_CTRL_AREA);
			angThrust.y = -m_mouseCMov[0] / MOUSE_CTRL_AREA;
			angThrust.x = m_mouseCMov[1] / MOUSE_CTRL_AREA;
		}
		
		if (m_flightControlState == CONTROL_FIXSPEED) {
			if (Pi::KeyState(SDLK_RETURN)) m_setSpeed += MAX(m_setSpeed*0.05, 1.0);
			if (Pi::KeyState(SDLK_RSHIFT)) m_setSpeed -= MAX(m_setSpeed*0.05, 1.0);
		}
		if (Pi::KeyState(SDLK_w)) SetThrusterState(ShipType::THRUSTER_REAR, 1.0f);
		if (Pi::KeyState(SDLK_s)) SetThrusterState(ShipType::THRUSTER_FRONT, 1.0f);
		if (Pi::KeyState(SDLK_2)) SetThrusterState(ShipType::THRUSTER_TOP, 1.0f);
		if (Pi::KeyState(SDLK_x)) SetThrusterState(ShipType::THRUSTER_BOTTOM, 1.0f);
		if (Pi::KeyState(SDLK_a)) SetThrusterState(ShipType::THRUSTER_LEFT, 1.0f);
		if (Pi::KeyState(SDLK_d)) SetThrusterState(ShipType::THRUSTER_RIGHT, 1.0f);

		if (Pi::KeyState(SDLK_SPACE) || (Pi::MouseButtonState(1) && Pi::MouseButtonState(3))) {
			if (Pi::worldView->GetCamType() == WorldView::CAM_REAR) SetGunState(1,1);
			else SetGunState(0,1);
		} else {
			SetGunState(0,0);
			SetGunState(1,0);
		}
		
		if (Pi::worldView->GetCamType() != WorldView::CAM_EXTERNAL) {
			if (Pi::KeyState(SDLK_LEFT)) angThrust.y += 1;
			if (Pi::KeyState(SDLK_RIGHT)) angThrust.y += -1;
			if (Pi::KeyState(SDLK_UP)) angThrust.x += -1;
			if (Pi::KeyState(SDLK_DOWN)) angThrust.x += 1;
		}
		if (Pi::KeyState(SDLK_q)) angThrust.z += 1;
		if (Pi::KeyState(SDLK_e)) angThrust.z -= 1;
		// dividing by time step so controls don't go totally mental when
		// used at 10x accel
		angThrust *= 1.0f/ta2;
		SetAngThrusterState(0, angThrust.x);
		SetAngThrusterState(1, angThrust.y);
		SetAngThrusterState(2, angThrust.z);
	}
}

