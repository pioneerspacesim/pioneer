#include "Pi.h"
#include "Player.h"
#include "Frame.h"
#include "WorldView.h"
#include "SpaceStationView.h"
#include "Serializer.h"
#include "Mission.h"
#include "Sound.h"
#include "ShipCpanel.h"

Player::Player(ShipType::Type shipType): Ship(shipType)
{
	m_mouseCMov[0] = m_mouseCMov[1] = 0;
	m_flightControlState = CONTROL_MANUAL;
	m_killCount = 0;
	m_knownKillCount = 0;
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
	wr_int(m_killCount);
	wr_int(m_knownKillCount);
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
	if (!IsOlderThan(6)) {
		m_killCount = rd_int();
		m_knownKillCount = rd_int();
	} else {
		m_killCount = 0;
		m_knownKillCount = 0;
	}
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
		Sound::BodyMakeNoise(this, Sound::SFX_WARNING, 1.0f);
	}
	return r;
}

void Player::TakeMission(Mission *m)
{
	m_missions.push_front(m);
	Pi::onPlayerMissionListChanged.emit();
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
	Ship::Render(viewCoords, viewTransform);
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

void Player::StaticUpdate(const float timeStep)
{
	Body *b;
	vector3d v;

	if (GetFlightState() == Ship::FLYING) {
		switch (m_flightControlState) {
		case CONTROL_FIXSPEED:
			if (Pi::GetView() == Pi::worldView) PollControls();
			b = (GetCombatTarget() ? GetCombatTarget() : GetNavTarget());
			v = vector3d(0, 0, -m_setSpeed);
			if (b) {
				matrix4x4d m;
				GetRotMatrix(m);
				v += m.InverseOf() * b->GetVelocityRelativeTo(this->GetFrame());
			}
			AIAccelToModelRelativeVelocity(v);
			break;
		case CONTROL_MANUAL:
			if (Pi::GetView() == Pi::worldView) PollControls();
			break;
		case CONTROL_AUTOPILOT:
			break;
		}
	} else {
		m_flightControlState = CONTROL_MANUAL;
		AIClearInstructions();
	}
	Ship::StaticUpdate(timeStep);
		
	/* Ship engine noise */
	static Uint32 sndev;
	float volBoth = 0.0f;
	volBoth += 0.5*GetThrusterState(ShipType::THRUSTER_REAR);
	volBoth += 0.5*GetThrusterState(ShipType::THRUSTER_FRONT);
	volBoth += 0.5*GetThrusterState(ShipType::THRUSTER_TOP);
	volBoth += 0.5*GetThrusterState(ShipType::THRUSTER_BOTTOM);
	
	float targetVol[2] = { volBoth, volBoth };
	targetVol[0] += 0.5*GetThrusterState(ShipType::THRUSTER_RIGHT);
	targetVol[1] += 0.5*GetThrusterState(ShipType::THRUSTER_LEFT);

	targetVol[0] = CLAMP(targetVol[0], 0.0f, 1.0f);
	targetVol[1] = CLAMP(targetVol[1], 0.0f, 1.0f);
	float dv_dt[2] = { 4.0f, 4.0f };
	if (!Sound::EventVolumeAnimate(sndev, targetVol, dv_dt)) {
		sndev = Sound::PlaySfx(Sound::SFX_ENGINES, 0.0f, 0.0f, true);
		Sound::EventVolumeAnimate(sndev, targetVol, dv_dt);
	}
}

#define MOUSE_CTRL_AREA		10.0f
#define MOUSE_RESTITUTION	0.75f

void Player::PollControls()
{
	int mouseMotion[2];
	double time_accel = Pi::GetTimeAccel();
	double invTimeAccel = 1.0 / time_accel;

	if ((time_accel == 0) || GetDockedWith() || Pi::player->IsDead() ||
	    (GetFlightState() != FLYING)) {
		return;
	}

	/* if flying */
	{
		ClearThrusterState();
		
		vector3f wantAngVel(0.0f);

		if (Pi::MouseButtonState(3)) {
			Pi::GetMouseMotion(mouseMotion);
			m_mouseCMov[0] += mouseMotion[0];
			m_mouseCMov[1] += mouseMotion[1];
			m_mouseCMov[0] = CLAMP(m_mouseCMov[0]*MOUSE_RESTITUTION, -MOUSE_CTRL_AREA, MOUSE_CTRL_AREA);
			m_mouseCMov[1] = CLAMP(m_mouseCMov[1]*MOUSE_RESTITUTION, -MOUSE_CTRL_AREA, MOUSE_CTRL_AREA);
			wantAngVel.y = -m_mouseCMov[0] / MOUSE_CTRL_AREA;
			wantAngVel.x = m_mouseCMov[1] / MOUSE_CTRL_AREA;
		}
		
		if (m_flightControlState == CONTROL_FIXSPEED) {
			if (Pi::KeyState(SDLK_RETURN)) m_setSpeed += MAX(m_setSpeed*0.05, 1.0);
			if (Pi::KeyState(SDLK_RSHIFT)) m_setSpeed -= MAX(m_setSpeed*0.05, 1.0);
		}

		if (Pi::KeyState(SDLK_i)) SetThrusterState(ShipType::THRUSTER_REAR, 1.0f);
		if (Pi::KeyState(SDLK_k)) SetThrusterState(ShipType::THRUSTER_FRONT, 1.0f);
		if (Pi::KeyState(SDLK_u)) SetThrusterState(ShipType::THRUSTER_TOP, 1.0f);
		if (Pi::KeyState(SDLK_o)) SetThrusterState(ShipType::THRUSTER_BOTTOM, 1.0f);
		if (Pi::KeyState(SDLK_j)) SetThrusterState(ShipType::THRUSTER_LEFT, 1.0f);
		if (Pi::KeyState(SDLK_l)) SetThrusterState(ShipType::THRUSTER_RIGHT, 1.0f);
		
		SetGunState(0,0);
		SetGunState(1,0);
		if (Pi::KeyState(SDLK_SPACE) || (Pi::MouseButtonState(1) && Pi::MouseButtonState(3))) {
				SetGunState(Pi::worldView->GetActiveWeapon(), 1);
		}

		if (Pi::worldView->GetCamType() != WorldView::CAM_EXTERNAL) {
			if (Pi::KeyState(SDLK_LEFT)) wantAngVel.y += 1.0;
			if (Pi::KeyState(SDLK_RIGHT)) wantAngVel.y += -1.0;
			if (Pi::KeyState(SDLK_UP)) wantAngVel.x += -1.0;
			if (Pi::KeyState(SDLK_DOWN)) wantAngVel.x += 1.0;
		}
		if (Pi::KeyState(SDLK_a)) wantAngVel.y += 1.0;
		if (Pi::KeyState(SDLK_d)) wantAngVel.y += -1.0;
		if (Pi::KeyState(SDLK_w)) wantAngVel.x += -1.0;
		if (Pi::KeyState(SDLK_s)) wantAngVel.x += 1.0;
		if (Pi::KeyState(SDLK_q)) wantAngVel.z += 1.0;
		if (Pi::KeyState(SDLK_e)) wantAngVel.z -= 1.0;


		for (int axis=0; axis<3; axis++) wantAngVel[axis] = CLAMP(wantAngVel[axis], -invTimeAccel, invTimeAccel);
		
		const float angThrustSoftness = Pi::KeyState(SDLK_LSHIFT) ? 10.0 : 50.0;
		
		AIModelCoordsMatchAngVel(wantAngVel, angThrustSoftness);
	}
}

