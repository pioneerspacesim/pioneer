// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PlayerShipController.h"
#include "Frame.h"
#include "Game.h"
#include "GameConfig.h"
#include "GameSaveError.h"
#include "KeyBindings.h"
#include "Pi.h"
#include "Player.h"
#include "Ship.h"
#include "Space.h"
#include "WorldView.h"
#include "core/OS.h"
#include "lua/LuaObject.h"

#include <algorithm>

PlayerShipController::PlayerShipController() :
	ShipController(),
	m_combatTarget(0),
	m_navTarget(0),
	m_setSpeedTarget(0),
	m_controlsLocked(false),
	m_invertMouse(false),
	m_mouseActive(false),
	m_disableMouseFacing(false),
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

	if (!InputBindings.primaryFire) {
		Error("PlayerShipController was not properly initialized!\n"
			  "You must call PlayerShipController::RegisterInputBindings before initializing a PlayerShipController");
	}

	m_connRotationDampingToggleKey = InputBindings.toggleRotationDamping->onPress.connect(
		sigc::mem_fun(this, &PlayerShipController::ToggleRotationDamping));

	m_fireMissileKey = InputBindings.secondaryFire->onPress.connect(
		sigc::mem_fun(this, &PlayerShipController::FireMissile));

	m_setSpeedMode = InputBindings.toggleSetSpeed->onPress.connect(
		sigc::mem_fun(this, &PlayerShipController::ToggleSetSpeedMode));
}

PlayerShipController::InputBinding PlayerShipController::InputBindings;

void PlayerShipController::RegisterInputBindings()
{
	using namespace KeyBindings;
	auto controlsPage = Pi::input->GetBindingPage("ShipControls");

	auto weaponsGroup = controlsPage->GetBindingGroup("Weapons");
	InputBindings.targetObject = Pi::input->AddActionBinding("BindTargetObject", weaponsGroup, ActionBinding(SDLK_y));
	InputBindings.primaryFire = Pi::input->AddActionBinding("BindPrimaryFire", weaponsGroup, ActionBinding(SDLK_SPACE));
	InputBindings.secondaryFire = Pi::input->AddActionBinding("BindSecondaryFire", weaponsGroup, ActionBinding(SDLK_m));

	auto flightGroup = controlsPage->GetBindingGroup("ShipOrient");
	InputBindings.pitch = Pi::input->AddAxisBinding("BindAxisPitch", flightGroup, AxisBinding(SDLK_k, SDLK_i));
	InputBindings.yaw = Pi::input->AddAxisBinding("BindAxisYaw", flightGroup, AxisBinding(SDLK_j, SDLK_l));
	InputBindings.roll = Pi::input->AddAxisBinding("BindAxisRoll", flightGroup, AxisBinding(SDLK_u, SDLK_o));
	InputBindings.killRot = Pi::input->AddActionBinding("BindKillRot", flightGroup, ActionBinding(SDLK_p, SDLK_x));
	InputBindings.toggleRotationDamping = Pi::input->AddActionBinding("BindToggleRotationDamping", flightGroup, ActionBinding(SDLK_v));

	auto thrustGroup = controlsPage->GetBindingGroup("ManualControl");
	InputBindings.thrustForward = Pi::input->AddAxisBinding("BindAxisThrustForward", thrustGroup, AxisBinding(SDLK_w, SDLK_s));
	InputBindings.thrustUp = Pi::input->AddAxisBinding("BindAxisThrustUp", thrustGroup, AxisBinding(SDLK_r, SDLK_f));
	InputBindings.thrustLeft = Pi::input->AddAxisBinding("BindAxisThrustLeft", thrustGroup, AxisBinding(SDLK_a, SDLK_d));
	InputBindings.thrustLowPower = Pi::input->AddActionBinding("BindThrustLowPower", thrustGroup, ActionBinding(SDLK_LSHIFT));

	auto speedGroup = controlsPage->GetBindingGroup("SpeedControl");
	InputBindings.speedControl = Pi::input->AddAxisBinding("BindSpeedControl", speedGroup, AxisBinding(SDLK_RETURN, SDLK_RSHIFT));
	InputBindings.toggleSetSpeed = Pi::input->AddActionBinding("BindToggleSetSpeed", speedGroup, ActionBinding(SDLK_v));
}

PlayerShipController::~PlayerShipController()
{
	m_connRotationDampingToggleKey.disconnect();
	m_fireMissileKey.disconnect();
}

void PlayerShipController::SaveToJson(Json &jsonObj, Space *space)
{
	Json playerShipControllerObj({}); // Create JSON object to contain player ship controller data.
	playerShipControllerObj["flight_control_state"] = m_flightControlState;
	playerShipControllerObj["set_speed"] = m_setSpeed;
	playerShipControllerObj["low_thrust_power"] = m_lowThrustPower;
	playerShipControllerObj["rotation_damping"] = m_rotationDamping;
	playerShipControllerObj["index_for_combat_target"] = space->GetIndexForBody(m_combatTarget);
	playerShipControllerObj["index_for_nav_target"] = space->GetIndexForBody(m_navTarget);
	playerShipControllerObj["index_for_set_speed_target"] = space->GetIndexForBody(m_setSpeedTarget);
	jsonObj["player_ship_controller"] = playerShipControllerObj; // Add player ship controller object to supplied object.
}

void PlayerShipController::LoadFromJson(const Json &jsonObj)
{
	try {
		Json playerShipControllerObj = jsonObj["player_ship_controller"];

		m_flightControlState = playerShipControllerObj["flight_control_state"];
		m_setSpeed = playerShipControllerObj["set_speed"];
		m_lowThrustPower = playerShipControllerObj["low_thrust_power"];
		m_rotationDamping = playerShipControllerObj["rotation_damping"];
		//figure out actual bodies in PostLoadFixup - after Space body index has been built
		m_combatTargetIndex = playerShipControllerObj["index_for_combat_target"];
		m_navTargetIndex = playerShipControllerObj["index_for_nav_target"];
		m_setSpeedTargetIndex = playerShipControllerObj["index_for_set_speed_target"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
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
	// have to use this function. SDL mouse position event is bugged in windows
	SDL_GetRelativeMouseState(mouseMotion + 0, mouseMotion + 1); // call to flush

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
			if (Frame::GetFrame(m_ship->GetFrame())->IsRotFrame())
				SetFlightControlState(CONTROL_FIXSPEED);
			else
				SetFlightControlState(CONTROL_MANUAL);
			m_setSpeed = 0.0;
			break;
		default: assert(0); break;
		}
	} else
		SetFlightControlState(CONTROL_MANUAL);

	//call autopilot AI, if active (also applies to set speed and heading lock modes)
	OS::EnableFPE();
	m_ship->AITimeStep(timeStep);
	OS::DisableFPE();
}

void PlayerShipController::CheckControlsLock()
{
	m_controlsLocked = (Pi::game->IsPaused() || Pi::player->IsDead() || (m_ship->GetFlightState() != Ship::FLYING) || Pi::IsConsoleActive() || (Pi::GetView() != Pi::game->GetWorldView())); //to prevent moving the ship in starmap etc.
}

vector3d PlayerShipController::GetMouseDir() const
{
	// translate from system to local frame
	return m_mouseDir * Frame::GetFrame(m_ship->GetFrame())->GetOrient();
}

// mouse wraparound control function
static double clipmouse(double cur, double inp)
{
	if (cur * cur > 0.7 && cur * inp > 0) return 0.0;
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
		m_ship->SetGunState(0, 0);
		m_ship->SetGunState(1, 0);

		// vector3d wantAngVel(0.0);
		double angThrustSoftness = 10.0;

		const float linearThrustPower = (InputBindings.thrustLowPower->IsActive() ? m_lowThrustPower : 1.0f);

		if (Pi::input->MouseButtonState(SDL_BUTTON_RIGHT)) {
			// use ship rotation relative to system, unchanged by frame transitions
			matrix3x3d rot = m_ship->GetOrientRelTo(Frame::GetFrame(m_ship->GetFrame())->GetNonRotFrame());
			if (!m_mouseActive && !m_disableMouseFacing) {
				m_mouseDir = -rot.VectorZ();
				m_mouseX = m_mouseY = 0;
				m_mouseActive = true;
				Pi::input->SetCapturingMouse(true);
			}
			vector3d objDir = m_mouseDir * rot;

			const double radiansPerPixel = 0.00002 * m_fovY;
			const int maxMotion = std::max(abs(mouseMotion[0]), abs(mouseMotion[1]));
			const double accel = Clamp(maxMotion / 4.0, 0.0, 90.0 / m_fovY);

			m_mouseX += mouseMotion[0] * accel * radiansPerPixel;
			double modx = clipmouse(objDir.x, m_mouseX);
			m_mouseX -= modx;

			const bool invertY = (Pi::input->IsMouseYInvert() ? !m_invertMouse : m_invertMouse);

			m_mouseY += mouseMotion[1] * accel * radiansPerPixel * (invertY ? -1 : 1);
			double mody = clipmouse(objDir.y, m_mouseY);
			m_mouseY -= mody;

			if (!is_zero_general(modx) || !is_zero_general(mody)) {
				matrix3x3d mrot = matrix3x3d::RotateY(modx) * matrix3x3d::RotateX(mody);
				m_mouseDir = (rot * (mrot * objDir)).Normalized();
			}
		} else {
			if (m_mouseActive)
				Pi::input->SetCapturingMouse(false);

			m_mouseActive = false;
		}

		if (m_flightControlState == CONTROL_FIXSPEED) {
			double oldSpeed = m_setSpeed;
			if (stickySpeedKey && !InputBindings.speedControl->IsActive())
				stickySpeedKey = false;

			if (!stickySpeedKey) {
				const double MAX_SPEED = 300000000;
				m_setSpeed += InputBindings.speedControl->GetValue() * std::max(std::abs(m_setSpeed) * 0.05, 1.0);
				m_setSpeed = Clamp(m_setSpeed, -MAX_SPEED, MAX_SPEED);

				if (((oldSpeed < 0.0) && (m_setSpeed >= 0.0)) ||
					((oldSpeed > 0.0) && (m_setSpeed <= 0.0))) {
					// flipped from going forward to backwards. make the speed 'stick' at zero
					// until the player lets go of the key and presses it again
					stickySpeedKey = true;
					m_setSpeed = 0;
				}
			}
		}

		if (InputBindings.thrustForward->IsActive())
			m_ship->SetThrusterState(2, -linearThrustPower * InputBindings.thrustForward->GetValue());
		if (InputBindings.thrustUp->IsActive())
			m_ship->SetThrusterState(1, linearThrustPower * InputBindings.thrustUp->GetValue());
		if (InputBindings.thrustLeft->IsActive())
			m_ship->SetThrusterState(0, -linearThrustPower * InputBindings.thrustLeft->GetValue());

		if (InputBindings.primaryFire->IsActive() || (Pi::input->MouseButtonState(SDL_BUTTON_LEFT) && Pi::input->MouseButtonState(SDL_BUTTON_RIGHT))) {
			//XXX worldview? madness, ask from ship instead
			m_ship->SetGunState(Pi::game->GetWorldView()->GetActiveWeapon(), 1);
		}

		vector3d wantAngVel = vector3d(
			InputBindings.pitch->GetValue(),
			InputBindings.yaw->GetValue(),
			InputBindings.roll->GetValue());

		if (InputBindings.killRot->IsActive()) SetFlightControlState(CONTROL_FIXHEADING_KILLROT);

		if (InputBindings.thrustLowPower->IsActive())
			angThrustSoftness = 50.0;

		if (wantAngVel.Length() >= 0.001 || force_rotation_damping || m_rotationDamping) {
			if (Pi::game->GetTimeAccel() != Game::TIMEACCEL_1X) {
				for (int axis = 0; axis < 3; axis++)
					wantAngVel[axis] = wantAngVel[axis] * Pi::game->GetInvTimeAccelRate();
			}

			m_ship->AIModelCoordsMatchAngVel(wantAngVel, angThrustSoftness);
		}

		if (m_mouseActive && !m_disableMouseFacing) m_ship->AIFaceDirection(GetMouseDir());
	}
}

bool PlayerShipController::IsAnyAngularThrusterKeyDown()
{
	return !Pi::IsConsoleActive() && (InputBindings.pitch->IsActive() || InputBindings.yaw->IsActive() || InputBindings.roll->IsActive());
}

bool PlayerShipController::IsAnyLinearThrusterKeyDown()
{
	return !Pi::IsConsoleActive() && (InputBindings.thrustForward->IsActive() || InputBindings.thrustLeft->IsActive() || InputBindings.thrustUp->IsActive());
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
	LuaObject<Ship>::CallMethod(Pi::player, "FireMissileAt", "any", static_cast<Ship *>(Pi::player->GetCombatTarget()));
}

void PlayerShipController::ToggleSetSpeedMode()
{
	if (GetFlightControlState() != CONTROL_FIXSPEED) {
		SetFlightControlState(CONTROL_FIXSPEED);
	} else {
		SetFlightControlState(CONTROL_MANUAL);
	}
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

void PlayerShipController::SetCombatTarget(Body *const target, bool setSpeedTo)
{
	if (setSpeedTo)
		m_setSpeedTarget = target;
	m_combatTarget = target;
}

void PlayerShipController::SetNavTarget(Body *const target)
{
	m_navTarget = target;
}

void PlayerShipController::SetSetSpeedTarget(Body *const target)
{
	m_setSpeedTarget = target;
}
