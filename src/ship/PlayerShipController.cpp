// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "PlayerShipController.h"
#include "Frame.h"
#include "Game.h"
#include "GameConfig.h"
#include "GameSaveError.h"
#include "Input.h"
#include "Json.h"
#include "Pi.h"
#include "Player.h"
#include "SDL_keycode.h"
#include "Ship.h"
#include "Space.h"
#include "SystemView.h"
#include "WorldView.h"
#include "core/OS.h"
#include "lua/LuaObject.h"
#include "ship/ShipController.h"
#include "Sensors.h"

#include <algorithm>

// so that we can summarize the controller action from all subsystems
struct PlayerShipController::TotalDesiredAction {
	vector3d desiredLinear = {};  // in ship's frame
	vector3d desiredAngular = {}; // in ship's local space
	vector3d angPower = vector3d(1.0);
	vector3d linPower = vector3d(1.0);
	// true means that desiredLinear contains speed, false means thrust
	bool desireLinVel = false;
	// false means that zero desireAngVel components are to be ignored
	// used for rotation damping off
	bool desireAngVelFull = false;
};

// static functions with access to private class members
struct PlayerShipController::Util {

	template <class T>
	static T sign(const T &val)
	{
		return (val > 0) - (val < 0);
	};

	// the speed of the ship at which it will be 'stationary' in the selected follow mode
	static vector3d GetFollowBaseVelocity(const PlayerShipController &c)
	{
		assert(c.m_flightControlState == CONTROL_FIXSPEED);
		vector3d refVel(0.0);
		if (c.m_followTarget) {
			refVel += c.m_followTarget->GetVelocityRelTo(c.m_ship->GetFrame());
			if (c.m_followMode == FOLLOW_ORI) {
				auto frm = Pi::player->GetFrame();
				auto pos = Pi::player->GetPosition();
				auto tpos = c.m_followTarget->GetPositionRelTo(frm);
				auto rvel = c.m_followTarget->GetAngVelocity();
				// need to calculate the tangential speed at the point where we are
				auto tvel = rvel.Cross(pos - tpos);
				refVel += tvel;
			}
		}
		return refVel;
	}

	// if we crossed 0 on change, set the flag and set cruise speed to 0
	// (made to stop at 0)
	static void ChangeCruiseSpeedConstrained(PlayerShipController &c, double delta)
	{
		double oldSpeed = c.m_cruiseSpeed;
		c.ChangeCruiseSpeed(delta);
		bool cross_0 = (((oldSpeed < 0.0) && (c.m_cruiseSpeed >= 0.0)) ||
			((oldSpeed > 0.0) && (c.m_cruiseSpeed <= 0.0)));
		if (cross_0) {
			c.m_stickySpeedKey = true;
			c.m_cruiseSpeed = 0.0;
		}
		// limit to light speed always
		double light_speed = 300000000;
		c.m_cruiseSpeed = Clamp(c.m_cruiseSpeed, -light_speed, light_speed);
	}

	static void SetCruiseSpeedFromActualVelocity(PlayerShipController &c)
	{
		vector3d shipVel = c.m_ship->GetVelocity() - GetFollowBaseVelocity(c);
		double speed;
		if (c.m_cruiseDirection == CRUISE_FWD) {
			speed = shipVel.Dot(-c.m_ship->GetOrient().VectorZ());
		} else { // CRUISE_UP
			speed = shipVel.Dot(c.m_ship->GetOrient().VectorY());
		}
		ChangeCruiseSpeedConstrained(c, speed - c.m_cruiseSpeed);
	}

	// substitute for AIFaceDirection, if we want to combine this action with other rotational needs
	static vector3d NeededAngVelocityToFaceDirection(PlayerShipController &c, const float timeStep, const vector3d &dir, const vector3d &angPower)
	{
		vector3d rot_vel(0.0);
		auto ship_dir = -c.m_ship->GetOrient().VectorZ();
		auto want_rot = abs(acos(dir.Dot(ship_dir)));
		// just take the smallest, thiese can be different if the user pokes the joystick
		double thrustPower = std::min({ angPower.x, angPower.y, angPower.z });
		if (want_rot > 0.00001) {
			auto max_accel = c.m_ship->GetShipType()->angThrust / c.m_ship->GetAngularInertia() * thrustPower;
			auto good_speed = sqrt(2 * max_accel * want_rot) * 0.95;
			auto frame_speed = want_rot / timeStep;
			auto tot_speed = std::min(good_speed, frame_speed / 2);
			rot_vel = (ship_dir.Cross(dir) * c.m_ship->GetOrient()).Normalized() * tot_speed;
		}
		return rot_vel;
	};

	static void LimitActualSpeed(PlayerShipController &c, const float timeStep, TotalDesiredAction &outParams)
	{
		if (!c.m_speedLimiterActive) return; // limiter is off
		// external forces and forces from thrusters
		vector3d &levels = outParams.desiredLinear; // assume that desiredLinear is a thrust
		vector3d f = levels * c.m_ship->GetPropulsion()->GetThrust(levels);
		f = c.m_ship->GetOrient() * f;
		f += c.m_ship->GetExternalForce();
		// have to predict the speed in the next frame, otherwise external forces
		// can cause flashes of thrusters
		// it also allows you to limit if the thrust increases speed, and release
		// if the thrust decreases speed
		vector3d nextFrameVel = c.m_ship->GetVelocity() + f * (timeStep / c.m_ship->GetMass());
		if (nextFrameVel.Length() > c.m_speedLimit) {
			// the limiter works - maintain the current speed
			outParams.desireLinVel = true; // now desiredLinear contains the velocity
			outParams.desiredLinear = nextFrameVel.Normalized() * (std::max(0.0, c.m_speedLimit));
		}
	}
};

REGISTER_INPUT_BINDING(PlayerShipController)
{
	using namespace InputBindings;
	auto controlsPage = Pi::input->GetBindingPage("ShipControls");

	auto weaponsGroup = controlsPage->GetBindingGroup("Weapons");
	input->AddActionBinding("BindTargetObject", weaponsGroup, Action({ SDLK_y }));
	input->AddActionBinding("BindCycleHostiles", weaponsGroup, Action({ SDLK_t }));
	input->AddActionBinding("BindPrimaryFire", weaponsGroup, Action({ SDLK_SPACE }));
	input->AddActionBinding("BindSecondaryFire", weaponsGroup, Action({ SDLK_m }));

	auto flightGroup = controlsPage->GetBindingGroup("ShipOrient");
	input->AddAxisBinding("BindAxisPitch", flightGroup, Axis({}, { SDLK_k }, { SDLK_i }));
	input->AddAxisBinding("BindAxisYaw", flightGroup, Axis({}, { SDLK_j }, { SDLK_l }));
	input->AddAxisBinding("BindAxisRoll", flightGroup, Axis({}, { SDLK_q }, { SDLK_e }));
	input->AddActionBinding("BindKillRot", flightGroup, Action({ SDLK_p }, { SDLK_x }));
	input->AddActionBinding("BindToggleRotationDamping", flightGroup, Action({ SDLK_v }));

	auto thrustGroup = controlsPage->GetBindingGroup("ManualControl");
	input->AddAxisBinding("BindAxisThrustForward", thrustGroup, Axis({}, { SDLK_w }, { SDLK_s }));
	input->AddAxisBinding("BindAxisThrustUp", thrustGroup, Axis({}, { SDLK_r }, { SDLK_f }));
	input->AddAxisBinding("BindAxisThrustLeft", thrustGroup, Axis({}, { SDLK_a }, { SDLK_d }));
	input->AddActionBinding("BindThrustLowPower", thrustGroup, Action({ SDLK_LSHIFT }));

	auto speedGroup = controlsPage->GetBindingGroup("SpeedControl");
	input->AddAxisBinding("BindSpeedControl", speedGroup, Axis({}, { SDLK_RETURN }, { SDLK_RSHIFT }));
	input->AddActionBinding("BindToggleCruise", speedGroup, Action({ SDLK_v }));
	input->AddActionBinding("BindToggleSpeedLimiter", speedGroup, Action({ SDLK_l, SDLK_RALT }));

	auto landingGroup = controlsPage->GetBindingGroup("LandingControl");
	input->AddActionBinding("BindToggleLandingGear", landingGroup, Action({ SDLK_n }));
	input->AddAxisBinding("BindControlLandingGear", landingGroup, Axis());
}

PlayerShipController::PlayerShipController() :
	ShipController(),
	InputBindings(Pi::input),
	m_combatTarget(0),
	m_navTarget(0),
	m_followTarget(0),
	m_invertMouse(false),
	m_mouseActive(false),
	m_disableMouseFacing(false),
	m_rotationDamping(true),
	m_mouseX(0.0),
	m_mouseY(0.0),
	m_cruiseSpeed(0.0),
	m_flightControlState(CONTROL_MANUAL),
	m_lowThrustPower(0.25), // note: overridden by the default value in GameConfig.cpp (DefaultLowThrustPower setting)
	m_mouseDir(0.0)
{
	const float deadzone = Pi::config->Float("JoystickDeadzone");
	m_joystickDeadzone = Clamp(deadzone, 0.01f, 1.0f); // do not use (deadzone * deadzone) as values are 0<>1 range, aka: 0.1 * 0.1 = 0.01 or 1% deadzone!!! Not what player asked for!
	m_fovY = Pi::config->Float("FOVVertical");
	m_lowThrustPower = Pi::config->Float("DefaultLowThrustPower");
	m_aimingSens = Pi::config->Float("AimSensitivity", 1.0f);

	InputBindings.RegisterBindings();
	Pi::input->AddInputFrame(&InputBindings);

	m_connRotationDampingToggleKey = InputBindings.toggleRotationDamping->onPressed.connect(
		sigc::mem_fun(this, &PlayerShipController::ToggleRotationDamping));

	m_fireMissileKey = InputBindings.secondaryFire->onPressed.connect(
		sigc::mem_fun(this, &PlayerShipController::FireMissile));

	m_toggleCruise = InputBindings.toggleCruise->onPressed.connect(
		sigc::mem_fun(this, &PlayerShipController::ToggleCruise));

	m_toggleSpeedLimiter = InputBindings.toggleSpeedLimiter->onPressed.connect(
		[this]() {
			this->SetSpeedLimiterActive(not this->IsSpeedLimiterActive());
		});

	m_selectTarget = InputBindings.targetObject->onPressed.connect (
		sigc::mem_fun(this, &PlayerShipController::SelectTarget));

	m_cycleHostiles = InputBindings.cycleHostiles->onPressed.connect (
		sigc::mem_fun(this, &PlayerShipController::CycleHostiles));

	m_toggleLandingGear = InputBindings.toggleLandingGear->onPressed.connect(
		sigc::mem_fun(this, &PlayerShipController::OnToggleLandingGear));
}

void PlayerShipController::InputBinding::RegisterBindings()
{
	targetObject = AddAction("BindTargetObject");
	cycleHostiles = AddAction("BindCycleHostiles");
	primaryFire = AddAction("BindPrimaryFire");
	secondaryFire = AddAction("BindSecondaryFire");

	pitch = AddAxis("BindAxisPitch");
	yaw = AddAxis("BindAxisYaw");
	roll = AddAxis("BindAxisRoll");

	killRot = AddAction("BindKillRot");
	toggleRotationDamping = AddAction("BindToggleRotationDamping");

	thrustForward = AddAxis("BindAxisThrustForward");
	thrustLeft = AddAxis("BindAxisThrustLeft");
	thrustUp = AddAxis("BindAxisThrustUp");
	thrustLowPower = AddAction("BindThrustLowPower");

	speedControl = AddAxis("BindSpeedControl");
	toggleCruise = AddAction("BindToggleCruise");
	toggleSpeedLimiter = AddAction("BindToggleSpeedLimiter");

	toggleLandingGear = AddAction("BindToggleLandingGear");
	controlLandingGear = AddAxis("BindControlLandingGear");
}

PlayerShipController::~PlayerShipController()
{
	Pi::input->RemoveInputFrame(&InputBindings);
}

void PlayerShipController::SaveToJson(Json &jsonObj, Space *space)
{
	Json playerShipControllerObj({}); // Create JSON object to contain player ship controller data.
	playerShipControllerObj["flight_control_state"] = m_flightControlState;
	playerShipControllerObj["cruise_speed"] = m_cruiseSpeed;
	playerShipControllerObj["cruise_direction"] = m_cruiseDirection;
	playerShipControllerObj["speed_limit"] = m_speedLimit;
	playerShipControllerObj["speed_limiter_active"] = m_speedLimiterActive;
	playerShipControllerObj["follow_mode"] = m_followMode;
	playerShipControllerObj["low_thrust_power"] = m_lowThrustPower;
	playerShipControllerObj["rotation_damping"] = m_rotationDamping;
	playerShipControllerObj["index_for_combat_target"] = space->GetIndexForBody(m_combatTarget);
	playerShipControllerObj["index_for_nav_target"] = space->GetIndexForBody(m_navTarget);
	playerShipControllerObj["index_for_follow_target"] = space->GetIndexForBody(m_followTarget);
	jsonObj["player_ship_controller"] = playerShipControllerObj; // Add player ship controller object to supplied object.
}

void PlayerShipController::LoadFromJson(const Json &jsonObj)
{
	try {
		Json playerShipControllerObj = jsonObj["player_ship_controller"];

		m_flightControlState = playerShipControllerObj["flight_control_state"];
		m_cruiseSpeed = playerShipControllerObj.value("cruise_speed", 0.0);
		m_cruiseDirection = playerShipControllerObj.value("cruise_direction", CRUISE_FWD);
		m_speedLimit = playerShipControllerObj.value("speed_limit", 0.0);
		m_speedLimiterActive = playerShipControllerObj.value("speed_limiter_active", false);
		m_followMode = playerShipControllerObj.value("follow_mode", FOLLOW_POS);
		m_lowThrustPower = playerShipControllerObj["low_thrust_power"];
		m_rotationDamping = playerShipControllerObj["rotation_damping"];
		//figure out actual bodies in PostLoadFixup - after Space body index has been built
		m_combatTargetIndex = playerShipControllerObj["index_for_combat_target"];
		m_navTargetIndex = playerShipControllerObj["index_for_nav_target"];
		m_followTargetIndex = playerShipControllerObj.value("index_for_follow_target", SDL_MAX_UINT32);
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

void PlayerShipController::PostLoadFixup(Space *space)
{
	m_combatTarget = space->GetBodyByIndex(m_combatTargetIndex);
	m_navTarget = space->GetBodyByIndex(m_navTargetIndex);
	SetFollowTarget(space->GetBodyByIndex(m_followTargetIndex));
}

void PlayerShipController::FlightAssist(const float timeStep, TotalDesiredAction &outParams)
{

	// in other modes it doesn't make sense (yet?)
	assert(m_flightControlState == CONTROL_FIXSPEED);

	assert(!outParams.desireLinVel && "assumed that no one has touched the linear speed yet");
	outParams.desireLinVel = true;
	outParams.desiredLinear = {};
	// someone could touch the angular speed already, or not
	if (!outParams.desireAngVelFull) {
		outParams.desireAngVelFull = true;
		outParams.desiredAngular = {};
	}
	vector3d &lin = outParams.desiredLinear;
	vector3d &rot = outParams.desiredAngular;


	// even under reduced thrust conditions, we have to calculate and
	// compensate for the external forces and inertial forces due to follow
	// using as much thrust as needed, up to 100%

	// external forces in ship's local space
	vector3d extf = m_ship->GetExternalForce() * m_ship->GetOrient();
	// the target's own acceleration (if it is, for example, a speeding ship)
	vector3d followingAccel(0.0);
	if (m_followTarget) {

		vector3d currVel = m_followTarget->GetVelocityRelTo(m_ship->GetFrame());
		followingAccel = currVel - m_followTargetPrevVel;
		m_followTargetPrevVel = currVel;

		// centripetal acceleration, due to following a rotating target
		if (m_followMode == FOLLOW_ORI) {
			vector3d av = m_followTarget->GetAngVelocity();
			if (av.LengthSqr() > 1e-5) {
				vector3d pos = m_followTarget->GetPositionRelTo(m_ship);
				// need a perpendicular to the axis of rotation from the position of the ship
				pos -= av.Normalized() * av.Normalized().Dot(pos);
				vector3d ac = m_followTarget->GetAngVelocity().LengthSqr() * pos;
				followingAccel += ac;
			}
		}
	}

	// subtract this from external forces, because in this case, external
	// forces help us to follow
	// remains is what we must compensate
	extf -= followingAccel * m_ship->GetMass() * m_ship->GetOrient();

	// thrust weaking (power ratio 0.0 .. 1.0)
	vector3d &power = outParams.linPower;
	const float linearThrustPower = (InputBindings.thrustLowPower->IsActive() ? m_lowThrustPower : 1.0f);

	// apply follow velocity
	vector3d base_vel = Util::GetFollowBaseVelocity(*this);
	lin += base_vel;

	// apply follow acceleration (as frame dv)
	vector3d add_vel = followingAccel * timeStep;
	lin += add_vel;

	// sync rotation for ORI
	if (m_followTarget && m_followMode == FOLLOW_ORI) {
		rot += m_followTarget->GetAngVelocity() * Pi::player->GetOrient();
	}

	// strafing:
	// in CRUISE_FWD mode, the longitudinal strafe changes the 'cruise speed'
	// in CRUISE_UP  mode, the vectical     strafe changes the 'cruise speed'
	if (IsAnyLinearThrusterKeyDown()) {

		vector3d strafe(0.0);												// requred velocity for strafe
		double maxStrafeSpeed = m_speedLimiterActive ? m_speedLimit : 1000; // m/s

		// Z
		if (!m_stickySpeedKey && InputBindings.thrustForward->IsActive()) {
			float itval = InputBindings.thrustForward->GetValue() * linearThrustPower;
			power.z = fabs(itval);
			double strafeZ = 0.0;
			if (GetCruiseDirection() == CRUISE_FWD) {
				if (!IsShipDrifting() && fabs(m_cruiseSpeed) > 1.0) {
					Util::SetCruiseSpeedFromActualVelocity(*this); // also can cross 0 and stick to it
				}
				if (!m_stickySpeedKey) {
					double accel = itval * m_ship->GetPropulsion()->GetAccel(itval < 0 ? THRUSTER_REVERSE : THRUSTER_FORWARD);
					accel -= (extf / m_ship->GetMass()).z;
					Util::ChangeCruiseSpeedConstrained(*this, accel * timeStep);
				}
			} else {
				strafeZ = maxStrafeSpeed * Util::sign(itval);
			}
			strafe -= m_ship->GetOrient().VectorZ() * strafeZ;
		}

		// Y
		if (!m_stickySpeedKey && InputBindings.thrustUp->IsActive()) {
			float itval = InputBindings.thrustUp->GetValue() * linearThrustPower;
			power.y = fabs(itval);
			double strafeY = 0.0;
			if (GetCruiseDirection() == CRUISE_UP) { // maintain speed along the vertical axis
				if (!IsShipDrifting() && fabs(m_cruiseSpeed) > 1.0) {
					Util::SetCruiseSpeedFromActualVelocity(*this);
				}
				if (!m_stickySpeedKey) {
					double accel = itval * m_ship->GetPropulsion()->GetAccel(itval < 0 ? THRUSTER_DOWN : THRUSTER_UP);
					accel += (extf / m_ship->GetMass()).y;
					Util::ChangeCruiseSpeedConstrained(*this, accel * timeStep);
				}
			} else {
				strafeY = maxStrafeSpeed * Util::sign(itval);
			}
			strafe += m_ship->GetOrient().VectorY() * strafeY;
		}

		// X
		if (InputBindings.thrustLeft->IsActive()) {
			float itval = InputBindings.thrustLeft->GetValue() * linearThrustPower;
			strafe += -m_ship->GetOrient().VectorX() * maxStrafeSpeed * Util::sign(itval);
			power.x = fabs(itval);
		}

		// total desired speed along the axes
		// note: "main cruise axls" is applied via cruise speed elsewhere
		lin += strafe;
	} else {
		// no linear thrust key down
		power.x = power.y = power.z = linearThrustPower;
	}

	// thrust-independent setting of cruise speed
	if (!m_stickySpeedKey && InputBindings.speedControl->IsActive()) {
		Util::ChangeCruiseSpeedConstrained(*this, InputBindings.speedControl->GetValue() * std::max(std::abs(m_cruiseSpeed) * 0.05, 1.0));
	}

	// apply speed limiter to cruise speed
	if (m_speedLimiterActive && fabs(m_cruiseSpeed) > m_speedLimit) {
		m_cruiseSpeed = m_speedLimit * Util::sign(m_cruiseSpeed);
	}

	// apply cruise speed
	switch (GetCruiseDirection()) {
	case CRUISE_FWD:
		lin += -m_ship->GetOrient().VectorZ() * m_cruiseSpeed;
		break;
	case CRUISE_UP:
		lin += m_ship->GetOrient().VectorY() * m_cruiseSpeed;
		break;
	default: assert(false && "Unknown cruise direction");
	}

	// if 'cruise speed' is stuck at 0, here we will free it
	if (m_stickySpeedKey && !InputBindings.speedControl->IsActive()) {
		if (GetCruiseDirection() == CRUISE_FWD) {
			if (!InputBindings.thrustForward->IsActive()) {
				m_stickySpeedKey = false;
			}
		} else { // CRUISE_UP
			if (!InputBindings.thrustUp->IsActive()) {
				m_stickySpeedKey = false;
			}
		}
	}

	// recalculate power limits, depending on external and inertial forces
	// because we want to use all available thrust to counter these forces
	// regardless of the set power limit
	// the speed that these forces will develop over the upcoming frame
	vector3d extfv = extf * timeStep / m_ship->GetMass();
	// predicted direction of change in speed (and burning thrusters)
	vector3d dvDir = (lin - m_ship->GetVelocity()) * m_ship->GetOrient() - extfv;
	vector3d maxThrust = m_ship->GetPropulsion()->GetThrust(dvDir); // Newtons, positive only
	// correction can only be in a greater value
	vector3d comp = vector3d(
		std::min(extf.x / maxThrust.x * Util::sign(dvDir.x), 0.0),
		std::min(extf.y / maxThrust.y * Util::sign(dvDir.y), 0.0),
		std::min(extf.z / maxThrust.z * Util::sign(dvDir.z), 0.0)); // fraction of thrust
	power.x = Clamp(power.x - comp.x, 0.0, 1.0);
	power.y = Clamp(power.y - comp.y, 0.0, 1.0);
	power.z = Clamp(power.z - comp.z, 0.0, 1.0);
}

void PlayerShipController::ApplyTotalAction(const TotalDesiredAction &params)
{
	// setting the desired speed has priority over the low-level thrust setting
	if (params.desireLinVel) {
		m_ship->AIMatchVel(params.desiredLinear, params.linPower);
	} else if (m_ship->m_launchLockTimeout <= 0) {
		m_ship->GetPropulsion()->SetLinThrusterState(params.desiredLinear);
	} else if (params.desiredLinear.LengthSqr() > 0) {
		// unlock launchlock in case of activation of linear thrust
		m_ship->m_launchLockTimeout = 0;
	}

	m_ship->AIMatchAngVelObjSpace(params.desiredAngular, params.angPower, !params.desireAngVelFull);
}

void PlayerShipController::StaticUpdate(const float timeStep)
{
	// call autopilot AI, if active
	if (m_ship->AIIsActive()) {
		m_ship->AITimeStep(timeStep);
		// the speed limiter should not work when the autopilot is working
		if (IsSpeedLimiterActive()) SetSpeedLimiterActive(false);
		return;
	}

	vector3d v;
	matrix4x4d m;
	TotalDesiredAction act;

	int mouseMotion[2];
	// have to use this function. SDL mouse position event is bugged in windows
	SDL_GetRelativeMouseState(mouseMotion + 0, mouseMotion + 1); // call to flush

	UpdateLandingGear();

	if (m_ship->GetFlightState() == Ship::FLYING) {
		switch (m_flightControlState) {
		case CONTROL_FIXSPEED:
			PollControls(timeStep, mouseMotion, act);
			FlightAssist(timeStep, act);
			ApplyTotalAction(act);
			break;
		case CONTROL_MANUAL:
			// maintain the thrust set by the station when undocking while the timeout lasts
			// but only in manual mode
			PollControls(timeStep, mouseMotion, act);
			Util::LimitActualSpeed(*this, timeStep, act);
			// synchronize the rotation in 'follow orient' mode
			if (m_followTarget && m_followMode == FOLLOW_ORI) {
				act.desiredAngular += m_followTarget->GetAngVelocity() * Pi::player->GetOrient();
			}
			//m_followMode == FOLLOW_ORI is when we try to match rotation speeds with a target
			//so all axes should be matched even with rotation damping off
			act.desireAngVelFull = m_rotationDamping || (m_followTarget && m_followMode == FOLLOW_ORI);
			ApplyTotalAction(act);
			break;
		case CONTROL_FIXHEADING_FORWARD:
		case CONTROL_FIXHEADING_BACKWARD:
		case CONTROL_FIXHEADING_NORMAL:
		case CONTROL_FIXHEADING_ANTINORMAL:
		case CONTROL_FIXHEADING_RADIALLY_INWARD:
		case CONTROL_FIXHEADING_RADIALLY_OUTWARD:
		case CONTROL_FIXHEADING_KILLROT:
			PollControls(timeStep, mouseMotion, act);
			Util::LimitActualSpeed(*this, timeStep, act);
			v = m_ship->GetVelocity().NormalizedSafe();
			if (m_flightControlState == CONTROL_FIXHEADING_BACKWARD ||
				m_flightControlState == CONTROL_FIXHEADING_ANTINORMAL)
				v = -v;
			if (m_flightControlState == CONTROL_FIXHEADING_NORMAL ||
				m_flightControlState == CONTROL_FIXHEADING_ANTINORMAL)
				v = v.Cross(m_ship->GetPosition().NormalizedSafe()).NormalizedSafe();
			if (m_flightControlState == CONTROL_FIXHEADING_RADIALLY_INWARD)
				v = -m_ship->GetPosition().NormalizedSafe();
			if (m_flightControlState == CONTROL_FIXHEADING_RADIALLY_OUTWARD)
				v = m_ship->GetPosition().NormalizedSafe();
			if (m_flightControlState == CONTROL_FIXHEADING_KILLROT) {
				v = -m_ship->GetOrient().VectorZ();
				if (m_ship->GetAngVelocity().Length() < 0.0001) // fixme magic number
					SetFlightControlState(CONTROL_MANUAL);
			}
			act.desiredAngular += Util::NeededAngVelocityToFaceDirection(*this, timeStep, v, act.angPower);
			ApplyTotalAction(act);
			break;
		case CONTROL_AUTOPILOT:
			Pi::game->RequestTimeAccel(Game::TIMEACCEL_1X);
			//			AIMatchVel(vector3d(0.0));			// just in case autopilot doesn't...
			// actually this breaks last timestep slightly in non-relative target cases
			m_ship->AIMatchAngVelObjSpace(vector3d(0.0));
			if (Frame::GetFrame(m_ship->GetFrame())->IsRotFrame())
				SetFlightControlState(CONTROL_FIXSPEED);
			else
				SetFlightControlState(CONTROL_MANUAL);
			m_cruiseSpeed = 0.0;
			break;
		default: assert(0); break;
		}
	} else {
		SetFlightControlState(CONTROL_MANUAL);

		// TODO: this is a bit monkey-patched, but calling from SetFlightControlState doesn't properly clear the mouse capture state.
		// Do it here so we properly react to becoming docked while holding the mouse button down
		if (m_ship->GetFlightState() == Ship::DOCKED && m_mouseActive) {
			Pi::input->SetCapturingMouse(false);
			m_mouseActive = false;
		}
	}
}

bool PlayerShipController::AreControlsLocked()
{
	return (Pi::game->IsPaused() || Pi::player->IsDead() || (m_ship->GetFlightState() != Ship::FLYING) || !InputBindings.active || (Pi::GetView() != Pi::game->GetWorldView() && Pi::GetView() != Pi::game->GetSystemView()));
}

vector3d PlayerShipController::GetMouseDir() const
{
	// translate from system to local frame
	return m_mouseDir * Frame::GetFrame(m_ship->GetFrame())->GetOrient();
}

// needs to run inside CameraContext::Begin/EndFrame();
vector3d PlayerShipController::GetMouseViewDir() const
{
	// orientation according to mouse
	matrix3x3d cam_rot = Pi::game->GetWorldView()->GetCameraContext()->GetCameraOrient();
	vector3d mouseDir = GetMouseDir() * cam_rot;
	if (m_invertMouse)
		mouseDir = -mouseDir;
	return (m_ship->GetPhysRadius() * 1.5) * mouseDir;
}

// mouse wraparound control function
static double clipmouse(double cur, double inp)
{
	if (cur * cur > 0.7 && cur * inp > 0) return 0.0;
	if (inp > 0.2) return 0.2;
	if (inp < -0.2) return -0.2;
	return inp;
}

void PlayerShipController::PollControls(const float timeStep, int *mouseMotion, TotalDesiredAction &outParams)
{
	if (AreControlsLocked()) return;

	const float thrustPower = (InputBindings.thrustLowPower->IsActive() ? m_lowThrustPower : 1.0f);
	outParams.angPower.x = outParams.angPower.y = outParams.angPower.z = thrustPower;
	// at the moment we do not work with angle thrusters directly at all
	outParams.desireAngVelFull = true;

	if (IsAnyLinearThrusterKeyDown()) {
		if (InputBindings.thrustForward->IsActive())
			outParams.desiredLinear.z = -thrustPower * InputBindings.thrustForward->GetValue();
		if (InputBindings.thrustUp->IsActive())
			outParams.desiredLinear.y = thrustPower * InputBindings.thrustUp->GetValue();
		if (InputBindings.thrustLeft->IsActive())
			outParams.desiredLinear.x = -thrustPower * InputBindings.thrustLeft->GetValue();
	}
	// only linear thrust ^ is allowed in the system map
	if (Pi::GetView() == Pi::game->GetSystemView()) return;

	m_ship->SetGunState(0, 0);
	m_ship->SetGunState(1, 0);

	if (Pi::input->MouseButtonState(SDL_BUTTON_RIGHT)) {
		// use ship rotation relative to system, unchanged by frame transitions
		matrix3x3d rot = m_ship->GetOrientRelTo(Frame::GetFrame(m_ship->GetFrame())->GetNonRotFrame());
		if (!m_mouseActive && !m_disableMouseFacing) {
			m_mouseDir = -rot.VectorZ();
			m_mouseX = m_mouseY = 0;
			m_mouseActive = true;
			Pi::input->SetCapturingMouse(true);
		} else if (m_followTarget && m_followMode == FOLLOW_ORI) {
			// adjust the direction of the mouse by the amount of change in the orientation of the reference body (if has)
			m_mouseDir = m_followTarget->GetOrient() * m_followTargetPrevOrient.Transpose() * m_mouseDir;
		}

		if (m_followTarget && m_followMode == FOLLOW_ORI) {
			// catch the change in the orientation of the followed body
			m_followTargetPrevOrient = m_followTarget->GetOrient();
		}

		vector3d objDir = m_mouseDir * rot;

		// TODO: this FOV value can be out-of-sync with the value used to render the frame
		// PlayerShipController likely needs to know about the camera being rendered through
		const double radiansPerPixel = 0.00002 * m_fovY;
		const int maxMotion = std::max(abs(mouseMotion[0]), abs(mouseMotion[1]));
		const double accel = Clamp(maxMotion / 4.0, 0.0, 90.0 / m_fovY);

		m_mouseX += mouseMotion[0] * accel * radiansPerPixel * m_aimingSens;
		double modx = clipmouse(objDir.x, m_mouseX);
		m_mouseX -= modx;

		const bool invertY = (Pi::input->IsMouseYInvert() ? !m_invertMouse : m_invertMouse);

		m_mouseY += mouseMotion[1] * accel * radiansPerPixel * (invertY ? -1 : 1) * m_aimingSens;
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

	if (InputBindings.primaryFire->IsActive() || (Pi::input->MouseButtonState(SDL_BUTTON_LEFT) && Pi::input->MouseButtonState(SDL_BUTTON_RIGHT))) {
		//XXX worldview? madness, ask from ship instead
		m_ship->SetGunState(Pi::game->GetWorldView()->GetActiveWeapon(), 1);
	}

	vector3d wantAngVel = vector3d(
		InputBindings.pitch->GetValue() * thrustPower,
		InputBindings.yaw->GetValue() * thrustPower,
		InputBindings.roll->GetValue() * thrustPower);

	// for any acceleration of time, the ship will turn at the same apparent speed
	if (Pi::game->GetTimeAccel() != Game::TIMEACCEL_1X) {
		for (int axis = 0; axis < 3; axis++)
			wantAngVel[axis] = wantAngVel[axis] * Pi::game->GetInvTimeAccelRate();
	}

	if (m_mouseActive && !m_disableMouseFacing) {
		// mouse facing
		wantAngVel += Util::NeededAngVelocityToFaceDirection(*this, timeStep, GetMouseDir(), outParams.angPower);
	}

	if (InputBindings.killRot->IsActive()) SetFlightControlState(CONTROL_FIXHEADING_KILLROT);

	outParams.desiredAngular = wantAngVel;

	if (IsAnyAngularThrusterKeyDown()) {
		// rotation speed limit identical to thrust power (almost)
		// and for 100% thrust speed limit is 1 rad/s, but
		// for wantAngVel ~ 0 angPower should be 1.0
		// also angPower should be 0.0 - 1.0
		auto angVelToThrust = [](double x, double value) { return x > 1e-5 ? Clamp(x, 0.0, 1.0) : value; };
		outParams.angPower.z = angVelToThrust(fabs(wantAngVel.z), thrustPower);
		// but for the mouse, we want to use the maximum available (or planned reduced) power
		if (!m_mouseActive || m_disableMouseFacing) {
			outParams.angPower.x = angVelToThrust(fabs(wantAngVel.x), thrustPower);
			outParams.angPower.y = angVelToThrust(fabs(wantAngVel.y), thrustPower);
		}
	}
}

bool PlayerShipController::IsAnyAngularThrusterKeyDown()
{
	return InputBindings.pitch->IsActive() || InputBindings.yaw->IsActive() || InputBindings.roll->IsActive();
}

bool PlayerShipController::IsAnyLinearThrusterKeyDown()
{
	return InputBindings.thrustForward->IsActive() || InputBindings.thrustLeft->IsActive() || InputBindings.thrustUp->IsActive();
}

void PlayerShipController::SetFlightControlState(FlightControlState s)
{
	if (m_flightControlState == s)
		return;

	m_flightControlState = s;
	m_ship->AIClearInstructions();
	//set desired velocity to current actual
	if (m_flightControlState == CONTROL_FIXSPEED) {
		// Speed is set to the projection of the velocity onto the target.
		Util::SetCruiseSpeedFromActualVelocity(*this);
	} else if (m_flightControlState == CONTROL_AUTOPILOT || (m_flightControlState == CONTROL_MANUAL && m_followMode == FOLLOW_POS)) {
		// FOLLOW_POS mode has no meaning in manual mode
		SetFollowTarget(nullptr);
	}

	onChangeFlightControlState.emit();
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

void PlayerShipController::OnToggleLandingGear()
{
	Pi::player->SetWheelState(!Pi::player->GetWheelState());
}

void PlayerShipController::UpdateLandingGear()
{
	const float value = InputBindings.controlLandingGear->GetValue();
	if (value == 0.0)
		return;

	const bool state = value > 0.0;
	if (Pi::player->GetWheelState() != state)
		Pi::player->SetWheelState(state);
}

static constexpr double MAX_SELECT_VIEW_ANGLE = DEG2RAD(3.0);
static constexpr double RELATIVE_DIST_EPSILON = 1.0/20.0;

//Same value as in data/pigui/modules/radar.lua. They should be in sync.
#define MAX_RADAR_SIZE 1000000000
//Value from pigui/views/game.lua. They should be in sync.
#define IN_SPACE_INDICATOR_SHIP_MAX_DISTANCE 1000000

static bool HierarchyAndDistanceComparator(const Space::BodyDist& bd1, const Space::BodyDist& bd2)
{
	//If the bodies are relatively close to each other from pleyers perspective
	if(abs(bd1.dist - bd2.dist)/(bd1.dist + bd2.dist) < RELATIVE_DIST_EPSILON) {
		FrameId fid =  bd2.body->GetFrame();
		Frame* f = nullptr;

		//check if second body is parent of first
		while((f = Frame::GetFrame(fid))) {

			if(f->GetBody() == bd1.body)
				return true;

			fid = f->GetParent();
		}

		fid =  bd1.body->GetFrame();
		f = nullptr;

		//check if first body is parent of second
		while((f = Frame::GetFrame(fid))) {
			if(f->GetBody() == bd2.body)
				return false;

			fid = f->GetParent();
		}
	}

	//if bodies are far enough from each other or not related
	return bd1.dist < bd2.dist;
}

void PlayerShipController::SelectTarget()
{
	// camera orientation and offset relative to the ship. This vector is already normalized, but for some reason
	// has wrong direction
	vector3d view_dir = Pi::game->GetWorldView()->shipView->GetCameraController()->GetOrient().VectorZ() * (-1.0);

	//The cocpit camera is not in the center of the ship, this has to be accounted for when looking to the side
	//or for tall ships
	vector3d camera_offset = Pi::game->GetWorldView()->shipView->GetCameraController()->GetPosition();

	std::vector<Space::BodyDist> bodyDistList =
		Pi::game->GetSpace()->BodiesInAngle(Pi::player, camera_offset, view_dir, cos(MAX_SELECT_VIEW_ANGLE));

	//sort by distance and body hierarchy
	std::sort(bodyDistList.begin(), bodyDistList.end(), HierarchyAndDistanceComparator);

	//Can't rely on GetCombatTarget() or GetNavTarget() to know what was previously selected
	static Body *prevSelectTarget = nullptr;
	Body *newTarget = nullptr;

	//With radar ships will be selected at much greter distance, the idea is that even though
	//you can't see them on screen overlay, they could be visible on radar
	int ship_detect_dist = m_ship->m_stats.radar_cap > 0 ? MAX_RADAR_SIZE : IN_SPACE_INDICATOR_SHIP_MAX_DISTANCE;

	//cycling
	for (auto it = bodyDistList.begin(); it != bodyDistList.end(); ++it) {

		if (it->body->GetType() == ObjectType::PROJECTILE) continue;
		if ((it->body->GetType() == ObjectType::SHIP || it->body->GetType() == ObjectType::CARGOBODY
			|| it->body->GetType() == ObjectType::HYPERSPACECLOUD)
			&& it->dist > ship_detect_dist)
			continue;

		//selecting first valid target, but only if different then already selected
		if(!newTarget) {
			if (it->body->IsType(ObjectType::SHIP) || it->body->IsType(ObjectType::MISSILE)) {

				if(it->body != GetCombatTarget())
					newTarget = it->body;
			}
			else {
				if(it->body != GetNavTarget())
					newTarget = it->body;
			}

		}

		//If there was target previously selected we keep rolling till we find it
		//and then select next one. If no prevSelectTarget found the first valid is taken
		if (prevSelectTarget) {
			if (prevSelectTarget == it->body) {
				prevSelectTarget = nullptr;
				//next valid body will be selected
			}
			continue;
		}

		if (it->body->IsType(ObjectType::SHIP) || it->body->IsType(ObjectType::MISSILE)) {

			if(it->body != GetCombatTarget()) {
				newTarget = it->body;
				break;
			}
		}
		else {
			if(it->body != GetNavTarget()) {

				newTarget = it->body;
				break;
			}
		}
	}

	prevSelectTarget = newTarget;

	//newTarget is really new - not the same as current combat or nav target
	//if such fresh target was not found then newTarget should be null so we will
	//be toggling in such case
	if (newTarget) {
		if (newTarget->IsType(ObjectType::SHIP) || newTarget->IsType(ObjectType::MISSILE))
			SetCombatTarget(newTarget);
		else
			SetNavTarget(newTarget);
	} else {
		if (GetCombatTarget())
			SetCombatTarget(nullptr);
		else
			SetNavTarget(nullptr);
	}
}

void PlayerShipController::CycleHostiles()
{
	Body* newTarget = Pi::player->GetSensors()->ChooseTarget(Sensors::CYCLE_HOSTILE, GetCombatTarget());
	SetCombatTarget(newTarget);
}

void PlayerShipController::ToggleCruise()
{
	if (m_ship->GetFlightState() != Ship::FLYING)
		return;

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

Body *PlayerShipController::GetFollowTarget() const
{
	return m_followTarget;
}

void PlayerShipController::SetCombatTarget(Body *const target, bool setFollowTo)
{
	if (setFollowTo)
		SetFollowTarget(target);

	m_combatTarget = target;
	onChangeTarget.emit();
}

void PlayerShipController::SetNavTarget(Body *const target)
{
	m_navTarget = target;
	onChangeTarget.emit();
}

void PlayerShipController::SetFollowTarget(Body *const target)
{
	m_followTarget = target;

	// Ensure we do not have sudden acceleration spikes when switching targets
	if (m_followTarget)
		m_followTargetPrevVel = m_followTarget->GetVelocityRelTo(m_ship->GetFrame());
	else
		m_followTargetPrevVel = vector3d();

	// TODO: not sure, do we actually need this? we are only changing the follow target
	onChangeTarget.emit();
}

void PlayerShipController::SetCruiseDirection(CruiseDirection dir)
{
	m_cruiseDirection = dir;
	if (m_flightControlState == CONTROL_FIXSPEED) Util::SetCruiseSpeedFromActualVelocity(*this);
}

// 'cruise speed' is pretty different from real speed, but not too much
bool PlayerShipController::IsShipDrifting()
{
	vector3d setVel;

	const double MIN_DIFFERENCE_SQ = 10.0 * 10.0; // m/s (squared)
	// if the gap is too big it's not a drift either
	const double MAX_DIFFERENCE_SQ = 1000.0 * 1000.0;

	vector3d factVel = m_ship->GetVelocity() - Util::GetFollowBaseVelocity(*this);

	switch (GetCruiseDirection()) {
	case CRUISE_FWD:
		setVel = -m_ship->GetOrient().VectorZ() * m_cruiseSpeed;
		break;
	case CRUISE_UP:
		setVel = m_ship->GetOrient().VectorY() * m_cruiseSpeed;
		break;
	default:
		assert(false && "Unknown cruise mode");
	}

	return (factVel - setVel).LengthSqr() > MIN_DIFFERENCE_SQ && (factVel - setVel).LengthSqr() < MAX_DIFFERENCE_SQ;
}
