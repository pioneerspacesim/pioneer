// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipController.h"
#include "Frame.h"
#include "Game.h"
#include "KeyBindings.h"
#include "Pi.h"
#include "Player.h"
#include "Ship.h"
#include "Space.h"
#include "WorldView.h"
#include "OS.h"

void ShipController::StaticUpdate(float timeStep)
{
	OS::EnableFPE();
	m_ship->AITimeStep(timeStep);
	OS::DisableFPE();
}

PlayerShipController::PlayerShipController() :
	ShipController(),
	m_combatTarget(0),
	m_navTarget(0),
	m_setSpeedTarget(0),
	m_controlsLocked(false),
	m_invertMouse(false),
	m_mouseActive(false),
	m_rotationDamping(true),
	m_mouseX(0.0),
	m_mouseY(0.0),
	m_setSpeed(0.0),
	m_flightControlState(CONTROL_MANUAL),
	m_lowThrustPower(0.25), // note: overridden by the default value in GameConfig.cpp (DefaultLowThrustPower setting)
	m_mouseDir(0.0)
{
	const float deadzone = Pi::config->Float("JoystickDeadzone");
	m_joystickDeadzone = Clamp(deadzone, 0.01f, 1.0f); // do not use (deadzone * deadzone) as values are 0<>1 range, aka: 0.1 * 0.1 = 0.01 or 1% deadzone!!! Not what player asked for!
	m_fovY = Pi::config->Float("FOVVertical");
	m_lowThrustPower = Pi::config->Float("DefaultLowThrustPower");

	m_connRotationDampingToggleKey = KeyBindings::toggleRotationDamping.onPress.connect(
			sigc::mem_fun(this, &PlayerShipController::ToggleRotationDamping));

	m_fireMissileKey = KeyBindings::fireMissile.onPress.connect(
			sigc::mem_fun(this, &PlayerShipController::FireMissile));

}

PlayerShipController::~PlayerShipController()
{
	m_connRotationDampingToggleKey.disconnect();
	m_fireMissileKey.disconnect();
}

void PlayerShipController::SaveToJson(Json::Value &jsonObj, Space *space)
{
	Json::Value playerShipControllerObj(Json::objectValue); // Create JSON object to contain player ship controller data.
	playerShipControllerObj["flight_control_state"] = static_cast<int>(m_flightControlState);
	playerShipControllerObj["set_speed"] = DoubleToStr(m_setSpeed);
	playerShipControllerObj["low_thrust_power"] = FloatToStr(m_lowThrustPower);
	playerShipControllerObj["rotation_damping"] = m_rotationDamping;
	playerShipControllerObj["index_for_combat_target"] = space->GetIndexForBody(m_combatTarget);
	playerShipControllerObj["index_for_nav_target"] = space->GetIndexForBody(m_navTarget);
	playerShipControllerObj["index_for_set_speed_target"] = space->GetIndexForBody(m_setSpeedTarget);
	jsonObj["player_ship_controller"] = playerShipControllerObj; // Add player ship controller object to supplied object.
}

void PlayerShipController::LoadFromJson(const Json::Value &jsonObj)
{
	if (!jsonObj.isMember("player_ship_controller")) throw SavedGameCorruptException();
	Json::Value playerShipControllerObj = jsonObj["player_ship_controller"];

	if (!playerShipControllerObj.isMember("flight_control_state")) throw SavedGameCorruptException();
	if (!playerShipControllerObj.isMember("set_speed")) throw SavedGameCorruptException();
	if (!playerShipControllerObj.isMember("low_thrust_power")) throw SavedGameCorruptException();
	if (!playerShipControllerObj.isMember("rotation_damping")) throw SavedGameCorruptException();
	if (!playerShipControllerObj.isMember("index_for_combat_target")) throw SavedGameCorruptException();
	if (!playerShipControllerObj.isMember("index_for_nav_target")) throw SavedGameCorruptException();
	if (!playerShipControllerObj.isMember("index_for_set_speed_target")) throw SavedGameCorruptException();

	m_flightControlState = static_cast<FlightControlState>(playerShipControllerObj["flight_control_state"].asInt());
	m_setSpeed = StrToDouble(playerShipControllerObj["set_speed"].asString());
	m_lowThrustPower = StrToFloat(playerShipControllerObj["low_thrust_power"].asString());
	m_rotationDamping = playerShipControllerObj["rotation_damping"].asBool();
	//figure out actual bodies in PostLoadFixup - after Space body index has been built
	m_combatTargetIndex = playerShipControllerObj["index_for_combat_target"].asInt();
	m_navTargetIndex = playerShipControllerObj["index_for_nav_target"].asInt();
	m_setSpeedTargetIndex = playerShipControllerObj["index_for_set_speed_target"].asInt();
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

	int mouseMotion[2];
	SDL_GetRelativeMouseState (mouseMotion+0, mouseMotion+1);	// call to flush

	// external camera mouselook
	if (Pi::MouseButtonState(SDL_BUTTON_MIDDLE)) {
            MoveableCameraController *mcc = static_cast<MoveableCameraController*>(Pi::game->GetWorldView()->GetCameraController());
            const double accel = 0.01; // XXX configurable?
            mcc->RotateLeft(mouseMotion[0] * accel);
            mcc->RotateUp(  mouseMotion[1] * accel);
            // only mouselook if the player presses both mmb and rmb
            mouseMotion[0] = 0;
            mouseMotion[1] = 0;
        }

	if (m_ship->GetFlightState() == Ship::FLYING) {
		switch (m_flightControlState) {
		case CONTROL_FIXSPEED:
			PollControls(timeStep, true, mouseMotion);
			if (IsAnyLinearThrusterKeyDown()) break;
			v = -m_ship->GetOrient().VectorZ() * m_setSpeed;
			if (m_setSpeedTarget) {
				v += m_setSpeedTarget->GetVelocityRelTo(m_ship->GetFrame());
			}
			m_ship->AIMatchVel(v);
			break;
		case CONTROL_FIXHEADING_FORWARD:
		case CONTROL_FIXHEADING_BACKWARD:
		case CONTROL_FIXHEADING_NORMAL:
		case CONTROL_FIXHEADING_ANTINORMAL:
		case CONTROL_FIXHEADING_RADIALLY_INWARD:
		case CONTROL_FIXHEADING_RADIALLY_OUTWARD:
		case CONTROL_FIXHEADING_KILLROT:
			PollControls(timeStep, true, mouseMotion);
			if (IsAnyAngularThrusterKeyDown()) break;
			v = m_ship->GetVelocity().NormalizedSafe();
			if (m_flightControlState == CONTROL_FIXHEADING_BACKWARD ||
			    m_flightControlState == CONTROL_FIXHEADING_ANTINORMAL)
				v = -v;
			if (m_flightControlState == CONTROL_FIXHEADING_NORMAL ||
			    m_flightControlState == CONTROL_FIXHEADING_ANTINORMAL)
				v = v.Cross(m_ship->GetPosition().NormalizedSafe());
			if (m_flightControlState == CONTROL_FIXHEADING_RADIALLY_INWARD)
				v = -m_ship->GetPosition().NormalizedSafe();
			if (m_flightControlState == CONTROL_FIXHEADING_RADIALLY_OUTWARD)
				v = m_ship->GetPosition().NormalizedSafe();
			if (m_flightControlState == CONTROL_FIXHEADING_KILLROT) {
				v = -m_ship->GetOrient().VectorZ();
				if (m_ship->GetAngVelocity().Length() < 0.0001) // fixme magic number
					SetFlightControlState(CONTROL_MANUAL);
			}

			m_ship->AIFaceDirection(v);
			break;
		case CONTROL_MANUAL:
			PollControls(timeStep, false, mouseMotion);
			break;
		case CONTROL_AUTOPILOT:
			if (m_ship->AIIsActive()) break;
			Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
//			AIMatchVel(vector3d(0.0));			// just in case autopilot doesn't...
						// actually this breaks last timestep slightly in non-relative target cases
			m_ship->AIMatchAngVelObjSpace(vector3d(0.0));
			if (m_ship->GetFrame()->IsRotFrame()) SetFlightControlState(CONTROL_FIXSPEED);
			else SetFlightControlState(CONTROL_MANUAL);
			m_setSpeed = 0.0;
			break;
		default: assert(0); break;
		}
	}
	else SetFlightControlState(CONTROL_MANUAL);

	//call autopilot AI, if active (also applies to set speed and heading lock modes)
	OS::EnableFPE();
	m_ship->AITimeStep(timeStep);
	OS::DisableFPE();
}

void PlayerShipController::CheckControlsLock()
{
	m_controlsLocked = (Pi::game->IsPaused())
		|| Pi::player->IsDead()
		|| (m_ship->GetFlightState() != Ship::FLYING)
		|| Pi::IsConsoleActive()
		|| (Pi::GetView() != Pi::game->GetWorldView()); //to prevent moving the ship in starmap etc.
}

// mouse wraparound control function
static double clipmouse(double cur, double inp)
{
	if (cur*cur > 0.7 && cur*inp > 0) return 0.0;
	if (inp > 0.2) return 0.2;
	if (inp < -0.2) return -0.2;
	return inp;
}

void PlayerShipController::PollControls(const float timeStep, const bool force_rotation_damping, int *mouseMotion)
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
		if (Pi::MouseButtonState(SDL_BUTTON_RIGHT))
		{
			const matrix3x3d &rot = m_ship->GetOrient();
			if (!m_mouseActive) {
				m_mouseDir = -rot.VectorZ();	// in world space
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
				matrix3x3d mrot = matrix3x3d::RotateY(modx) * matrix3x3d::RotateX(mody);
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
				m_ship->SetGunState(Pi::game->GetWorldView()->GetActiveWeapon(), 1);
		}

		if (KeyBindings::yawLeft.IsActive()) wantAngVel.y += 1.0;
		if (KeyBindings::yawRight.IsActive()) wantAngVel.y += -1.0;
		if (KeyBindings::pitchDown.IsActive()) wantAngVel.x += -1.0;
		if (KeyBindings::pitchUp.IsActive()) wantAngVel.x += 1.0;
		if (KeyBindings::rollLeft.IsActive()) wantAngVel.z += 1.0;
		if (KeyBindings::rollRight.IsActive()) wantAngVel.z -= 1.0;
		if (KeyBindings::killRot.IsActive()) SetFlightControlState(CONTROL_FIXHEADING_KILLROT);

		if (KeyBindings::thrustLowPower.IsActive())
			angThrustSoftness = 50.0;

		vector3d changeVec;
		changeVec.x = KeyBindings::pitchAxis.GetValue();
		changeVec.y = KeyBindings::yawAxis.GetValue();
		changeVec.z = KeyBindings::rollAxis.GetValue();

		// Deadzone per-axis with normalisation
		const float dz = m_joystickDeadzone;
		for (int axis=0; axis<3; axis++) {
			if (fabs(changeVec[axis]) < dz) {
				// no input
				changeVec[axis] = 0.0f;
			} else {
				// subtract deadzone and re-normalise to full range
				changeVec[axis] = (changeVec[axis] - dz) / (1.0f - dz);
			}
		}
		
		wantAngVel += changeVec;

		if (wantAngVel.Length() >= 0.001 || force_rotation_damping || m_rotationDamping) {
			if (Pi::game->GetTimeAccel()!=Game::TIMEACCEL_1X) {
				for (int axis=0; axis<3; axis++)
					wantAngVel[axis] = wantAngVel[axis] * Pi::game->GetInvTimeAccelRate();
			}

			m_ship->AIModelCoordsMatchAngVel(wantAngVel, angThrustSoftness);
		}

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
			// Speed is set to the projection of the velocity onto the target.

			vector3d shipVel = m_setSpeedTarget ?
				// Ship's velocity with respect to the target, in current frame's coordinates
				-m_setSpeedTarget->GetVelocityRelTo(m_ship) :
				// Ship's velocity with respect to current frame
				m_ship->GetVelocity();

			// A change from Manual to Set Speed never sets a negative speed.
			m_setSpeed = std::max(shipVel.Dot(-m_ship->GetOrient().VectorZ()), 0.0);
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

void PlayerShipController::SetRotationDamping(bool enabled)
{
	if (enabled != m_rotationDamping) {
		m_rotationDamping = enabled;
		onRotationDampingChanged.emit();
	}
}

void PlayerShipController::ToggleRotationDamping()
{
	SetRotationDamping(!GetRotationDamping());
}

void PlayerShipController::FireMissile()
{
	if (!Pi::player->GetCombatTarget())
		return;
	LuaObject<Ship>::CallMethod(Pi::player, "FireMissileAt", "any", static_cast<Ship*>(Pi::player->GetCombatTarget()));
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
