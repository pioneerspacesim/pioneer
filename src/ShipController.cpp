#include "ShipController.h"
#include "Frame.h"
#include "Game.h"
#include "KeyBindings.h"
#include "Pi.h"
#include "Player.h"
#include "Ship.h"
#include "Space.h"
#include "WorldView.h"

void ShipController::StaticUpdate(float timeStep)
{
	m_ship->AITimeStep(timeStep);
}

PlayerShipController::PlayerShipController() :
	ShipController(),
	m_combatTarget(0),
	m_navTarget(0),
	m_setSpeedTarget(0),
	m_controlsLocked(false),
	m_invertMouse(false),
	m_mouseActive(false),
	m_mouseX(0.0),
	m_mouseY(0.0),
	m_setSpeed(0.0),
	m_flightControlState(CONTROL_MANUAL),
	m_lowThrustPower(0.25), // note: overridden by the default value in GameConfig.cpp (DefaultLowThrustPower setting)
	m_mouseDir(0.0)
{
	float deadzone = Pi::config->Float("JoystickDeadzone");
	m_joystickDeadzone = deadzone * deadzone;
	m_fovY = Pi::config->Float("FOVVertical");
	m_lowThrustPower = Pi::config->Float("DefaultLowThrustPower");
}

PlayerShipController::~PlayerShipController()
{

} 

void PlayerShipController::Save(Serializer::Writer &wr, Space *space)
{
	wr.Int32(static_cast<int>(m_flightControlState));
	wr.Double(m_setSpeed);
	wr.Float(m_lowThrustPower);
	wr.Int32(space->GetIndexForBody(m_combatTarget));
	wr.Int32(space->GetIndexForBody(m_navTarget));
	wr.Int32(space->GetIndexForBody(m_setSpeedTarget));
}

void PlayerShipController::Load(Serializer::Reader &rd)
{
	m_flightControlState = static_cast<FlightControlState>(rd.Int32());
	m_setSpeed = rd.Double();
	m_lowThrustPower = rd.Float();
	//figure out actual bodies in PostLoadFixup - after Space body index has been built
	m_combatTargetIndex = rd.Int32();
	m_navTargetIndex = rd.Int32();
	m_setSpeedTargetIndex = rd.Int32();
}

void PlayerShipController::PostLoadFixup(Space *space)
{
	m_combatTarget = space->GetBodyByIndex(m_combatTargetIndex);
	m_navTarget = space->GetBodyByIndex(m_navTargetIndex);
	m_setSpeedTarget = space->GetBodyByIndex(m_setSpeedTargetIndex);
}

void PlayerShipController::StaticUpdate(const float timeStep)
{
	vector3d v;
	matrix4x4d m;

	if (m_ship->GetFlightState() == Ship::FLYING) {
		switch (m_flightControlState) {
		case CONTROL_FIXSPEED:
			PollControls(timeStep);
			if (IsAnyLinearThrusterKeyDown()) break;
			m_ship->GetRotMatrix(m);
			v = m * vector3d(0, 0, -m_setSpeed);
			if (m_setSpeedTarget) {
				v += m_setSpeedTarget->GetVelocityRelTo(m_ship->GetFrame());
			}
			m_ship->AIMatchVel(v);
			break;
		case CONTROL_FIXHEADING_FORWARD:
		case CONTROL_FIXHEADING_BACKWARD:
			PollControls(timeStep);
			if (IsAnyAngularThrusterKeyDown()) break;
			v = m_ship->GetVelocity().NormalizedSafe();
			if (m_flightControlState == CONTROL_FIXHEADING_BACKWARD)
				v = -v;
			m_ship->AIFaceDirection(v);
			break;
		case CONTROL_MANUAL:
			PollControls(timeStep);
			break;
		case CONTROL_AUTOPILOT:
			if (m_ship->AIIsActive()) break;
			Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
//			AIMatchVel(vector3d(0.0));			// just in case autopilot doesn't...
						// actually this breaks last timestep slightly in non-relative target cases
			m_ship->AIMatchAngVelObjSpace(vector3d(0.0));
			if (m_ship->GetFrame()->IsRotatingFrame()) SetFlightControlState(CONTROL_FIXSPEED);
			else SetFlightControlState(CONTROL_MANUAL);
			m_setSpeed = 0.0;
			break;
		default: assert(0); break;
		}
	}
	else SetFlightControlState(CONTROL_MANUAL);

	//call autopilot AI, if active (also applies to set speed and heading lock modes)
	m_ship->AITimeStep(timeStep);
}

void PlayerShipController::CheckControlsLock()
{
	m_controlsLocked = (Pi::game->IsPaused())
		|| Pi::player->IsDead()
		|| (m_ship->GetFlightState() != Ship::FLYING)
		|| Pi::IsConsoleActive()
		|| (Pi::GetView() != Pi::worldView); //to prevent moving the ship in starmap etc.
}

// mouse wraparound control function
static double clipmouse(double cur, double inp)
{
	if (cur*cur > 0.7 && cur*inp > 0) return 0.0;
	if (inp > 0.2) return 0.2;
	if (inp < -0.2) return -0.2;
	return inp;
}

void PlayerShipController::PollControls(const float timeStep)
{
	static bool stickySpeedKey = false;

	CheckControlsLock();
	if (m_controlsLocked) return;

	// if flying 
	{
		m_ship->ClearThrusterState();
		m_ship->SetGunState(0,0);
		m_ship->SetGunState(1,0);

		vector3d wantAngVel(0.0);
		double angThrustSoftness = 10.0;

		const float linearThrustPower = (KeyBindings::thrustLowPower.IsActive() ? m_lowThrustPower : 1.0f);

		// have to use this function. SDL mouse position event is bugged in windows
		int mouseMotion[2];
		SDL_GetRelativeMouseState (mouseMotion+0, mouseMotion+1);	// call to flush
		if (Pi::MouseButtonState(SDL_BUTTON_RIGHT))
		{
			matrix4x4d rot; m_ship->GetRotMatrix(rot);
			if (!m_mouseActive) {
				m_mouseDir = vector3d(-rot[8],-rot[9],-rot[10]);	// in world space
				m_mouseX = m_mouseY = 0;
				m_mouseActive = true;
			}
			vector3d objDir = m_mouseDir * rot;

			const double radiansPerPixel = 0.00002 * m_fovY;
			const int maxMotion = std::max(abs(mouseMotion[0]), abs(mouseMotion[1]));
			const double accel = Clamp(maxMotion / 4.0, 0.0, 90.0 / m_fovY);

			m_mouseX += mouseMotion[0] * accel * radiansPerPixel;
			double modx = clipmouse(objDir.x, m_mouseX);
			m_mouseX -= modx;

			const bool invertY = (Pi::IsMouseYInvert() ? !m_invertMouse : m_invertMouse);

			m_mouseY += mouseMotion[1] * accel * radiansPerPixel * (invertY ? -1 : 1);
			double mody = clipmouse(objDir.y, m_mouseY);
			m_mouseY -= mody;

			if(!is_zero_general(modx) || !is_zero_general(mody)) {
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

		if (KeyBindings::thrustForward.IsActive()) m_ship->SetThrusterState(2, -linearThrustPower);
		if (KeyBindings::thrustBackwards.IsActive()) m_ship->SetThrusterState(2, linearThrustPower);
		if (KeyBindings::thrustUp.IsActive()) m_ship->SetThrusterState(1, linearThrustPower);
		if (KeyBindings::thrustDown.IsActive()) m_ship->SetThrusterState(1, -linearThrustPower);
		if (KeyBindings::thrustLeft.IsActive()) m_ship->SetThrusterState(0, -linearThrustPower);
		if (KeyBindings::thrustRight.IsActive()) m_ship->SetThrusterState(0, linearThrustPower);

		if (KeyBindings::fireLaser.IsActive() || (Pi::MouseButtonState(SDL_BUTTON_LEFT) && Pi::MouseButtonState(SDL_BUTTON_RIGHT))) {
				//XXX worldview? madness, ask from ship instead
				m_ship->SetGunState(Pi::worldView->GetActiveWeapon(), 1);
		}

		if (KeyBindings::yawLeft.IsActive()) wantAngVel.y += 1.0;
		if (KeyBindings::yawRight.IsActive()) wantAngVel.y += -1.0;
		if (KeyBindings::pitchDown.IsActive()) wantAngVel.x += -1.0;
		if (KeyBindings::pitchUp.IsActive()) wantAngVel.x += 1.0;
		if (KeyBindings::rollLeft.IsActive()) wantAngVel.z += 1.0;
		if (KeyBindings::rollRight.IsActive()) wantAngVel.z -= 1.0;

		if (KeyBindings::thrustLowPower.IsActive())
			angThrustSoftness = 50.0;

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
		
		m_ship->AIModelCoordsMatchAngVel(wantAngVel, angThrustSoftness);
		if (m_mouseActive) m_ship->AIFaceDirection(m_mouseDir);
	}
}

bool PlayerShipController::IsAnyAngularThrusterKeyDown()
{
	return !Pi::IsConsoleActive() && (
		KeyBindings::pitchUp.IsActive()   ||
		KeyBindings::pitchDown.IsActive() ||
		KeyBindings::yawLeft.IsActive()   ||
		KeyBindings::yawRight.IsActive()  ||
		KeyBindings::rollLeft.IsActive()  ||
		KeyBindings::rollRight.IsActive()
	);
}

bool PlayerShipController::IsAnyLinearThrusterKeyDown()
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

void PlayerShipController::SetFlightControlState(FlightControlState s)
{
	if (m_flightControlState != s) {
		m_flightControlState = s;
		m_ship->AIClearInstructions();
		//set desired velocity to current actual
		if (m_flightControlState == CONTROL_FIXSPEED) {
			m_setSpeed = m_setSpeedTarget ? m_ship->GetVelocityRelTo(m_setSpeedTarget).Length() : m_ship->GetVelocity().Length();
		}
		//XXX global stuff
		Pi::onPlayerChangeFlightControlState.emit();
	}
}

void PlayerShipController::SetLowThrustPower(float power)
{
	assert((power >= 0.0f) && (power <= 1.0f));
	m_lowThrustPower = power;
}

Body *PlayerShipController::GetCombatTarget() const
{
	return m_combatTarget;
}

Body *PlayerShipController::GetNavTarget() const
{
	return m_navTarget;
}

Body *PlayerShipController::GetSetSpeedTarget() const
{
	return m_setSpeedTarget;
}

void PlayerShipController::SetCombatTarget(Body* const target, bool setSpeedTo)
{
	if (setSpeedTo)
		m_setSpeedTarget = target;
	else if (m_setSpeedTarget == m_combatTarget)
		m_setSpeedTarget = 0;
	m_combatTarget = target;
}

void PlayerShipController::SetNavTarget(Body* const target, bool setSpeedTo)
{
	if (setSpeedTo)
		m_setSpeedTarget = target;
	else if (m_setSpeedTarget == m_navTarget)
		m_setSpeedTarget = 0;
	m_navTarget = target;		
}
