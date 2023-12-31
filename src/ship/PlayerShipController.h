// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Input.h"
#include "ShipController.h"
#include "ConnectionTicket.h"

#include "vector3.h"
#include "matrix3x3.h"

// autopilot AI + input
class PlayerShipController : public ShipController {
public:
	PlayerShipController();
	~PlayerShipController();

	Type GetType() override { return PLAYER; }
	void SaveToJson(Json &jsonObj, Space *s) override;
	void LoadFromJson(const Json &jsonObj) override;
	void PostLoadFixup(Space *s) override;
	void StaticUpdate(float timeStep) override;
	bool IsMouseActive() const { return m_mouseActive; }
	void SetDisableMouseFacing(bool disabled) { m_disableMouseFacing = disabled; }
	double GetCruiseSpeed() const override { return m_cruiseSpeed; }
	void ChangeCruiseSpeed(double delta) override { m_cruiseSpeed += delta; }
	FlightControlState GetFlightControlState() const override { return m_flightControlState; }
	vector3d GetMouseDir() const; // in local frame

	// Return the current mouse direction in camera space.
	vector3d GetMouseViewDir() const;
	void SetMouseForRearView(bool enable) { m_invertMouse = enable; }
	void SetFlightControlState(FlightControlState s) override;
	float GetLowThrustPower() const { return m_lowThrustPower; }
	void SetLowThrustPower(float power);

	bool GetRotationDamping() const { return m_rotationDamping; }
	void SetRotationDamping(bool enabled);
	void ToggleRotationDamping();
	void FireMissile();
	void ToggleCruise();
	void SelectTarget();
	void CycleHostiles();



	//targeting
	//XXX AI should utilize one or more of these
	Body *GetCombatTarget() const;
	Body *GetNavTarget() const;
	Body *GetFollowTarget() const override;
	void SetCombatTarget(Body *const target, bool setFollowTo = false);
	void SetNavTarget(Body *const target);
	void SetFollowTarget(Body *const target);

	sigc::signal<void> onRotationDampingChanged;
	sigc::signal<void> onChangeTarget;
	sigc::signal<void> onChangeFlightControlState;

	enum CruiseDirection { // <enum scope='PlayerShipController' name=CruiseDirection public>
		CRUISE_FWD,
		CRUISE_UP
	};

	enum FollowMode { // <enum scope='PlayerShipController' name=FollowMode public>
		FOLLOW_POS,
		FOLLOW_ORI
	};

	void SetCruiseDirection(CruiseDirection mode);
	CruiseDirection GetCruiseDirection() const { return m_cruiseDirection; }
	void SetFollowMode(FollowMode mode) { m_followMode = mode; }
	FollowMode GetFollowMode() const { return m_followMode; }
	void SetSpeedLimit(double limit) { m_speedLimit = limit; }
	double GetSpeedLimit() { return m_speedLimit; }
	void SetSpeedLimiterActive(bool active) { m_speedLimiterActive = active; }
	bool IsSpeedLimiterActive() const { return m_speedLimiterActive; }
	bool IsShipDrifting(); // 'setted' speed is very different from real speed

private:
	struct InputBinding : public Input::InputFrame {
		using InputFrame::InputFrame;

		// Weapons
		Action *targetObject;
		Action *cycleHostiles;
		Action *primaryFire;
		Action *secondaryFire;

		// Flight
		Axis *pitch;
		Axis *yaw;
		Axis *roll;
		Action *killRot;
		Action *toggleRotationDamping;

		// Manual Control
		Axis *thrustForward;
		Axis *thrustUp;
		Axis *thrustLeft;
		Action *thrustLowPower;

		// Speed Control
		Axis *speedControl;
		Action *toggleCruise;
		Action *toggleSpeedLimiter;

		// Landing Controls
		Action *toggleLandingGear;
		Axis *controlLandingGear;

		void RegisterBindings() override;
	} InputBindings;

	// cumulative action from the input and flight assistance
	struct TotalDesiredAction;
	// static functions with access to private members
	struct Util;

	// Poll controls, set gun states
	void PollControls(float timeStep, int *mouseMotion, TotalDesiredAction &outParams);
	// desired speed of linear and rotary movement, calculated by flight control assistance
	void FlightAssist(const float timeStep, TotalDesiredAction &outParams);
	// send a control request to propulsion
	void ApplyTotalAction(const TotalDesiredAction &params);

	void OnToggleLandingGear();
	void UpdateLandingGear();

	// FIXME: separate the propusion controller from the input system, pass in wanted velocity correction directly.
	friend class Propulsion;

	double m_speedLimit = 0.0;
	bool m_speedLimiterActive = false;
	bool IsAnyAngularThrusterKeyDown();
	bool IsAnyLinearThrusterKeyDown();
	//do a variety of checks to see if input is allowed
	bool AreControlsLocked();
	Body *m_combatTarget;
	Body *m_navTarget;
	Body *m_followTarget;
	bool m_invertMouse; // used for rear view, *not* for invert Y-axis option (which is Pi::input->IsMouseYInvert)
	bool m_mouseActive;
	bool m_disableMouseFacing;
	bool m_rotationDamping;
	bool m_stickySpeedKey = false; // helps cruise speed sticks to 0 when it crosses it
	matrix3x3d m_followTargetPrevOrient;
	vector3d m_followTargetPrevVel;
	double m_mouseX;
	double m_mouseY;
	double m_cruiseSpeed;
	FlightControlState m_flightControlState;
	float m_fovY; //for mouse acceleration adjustment
	float m_joystickDeadzone;
	float m_lowThrustPower;
	float m_aimingSens;
	int m_combatTargetIndex; //for PostLoadFixUp
	int m_navTargetIndex;
	int m_followTargetIndex;
	vector3d m_mouseDir;

	FollowMode m_followMode = FOLLOW_POS;
	CruiseDirection m_cruiseDirection = CRUISE_FWD;

	ConnectionTicket m_connRotationDampingToggleKey;
	ConnectionTicket m_fireMissileKey;
	ConnectionTicket m_toggleCruise;
	ConnectionTicket m_toggleSpeedLimiter;
	ConnectionTicket m_selectTarget;
	ConnectionTicket m_cycleHostiles;
	ConnectionTicket m_toggleLandingGear;
};
