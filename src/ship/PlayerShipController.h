// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "Input.h"
#include "ShipController.h"

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
	// Poll controls, set thruster states, gun states and target velocity
	void PollControls(float timeStep, const bool force_rotation_damping, int *mouseMotion);
	bool IsMouseActive() const { return m_mouseActive; }
	void SetDisableMouseFacing(bool disabled) { m_disableMouseFacing = disabled; }
	double GetSetSpeed() const override { return m_setSpeed; }
	void ChangeSetSpeed(double delta) override { m_setSpeed += delta; }
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
	void ToggleSetSpeedMode();

	//targeting
	//XXX AI should utilize one or more of these
	Body *GetCombatTarget() const;
	Body *GetNavTarget() const;
	Body *GetSetSpeedTarget() const override;
	void SetCombatTarget(Body *const target, bool setSpeedTo = false);
	void SetNavTarget(Body *const target);
	void SetSetSpeedTarget(Body *const target);

	sigc::signal<void> onRotationDampingChanged;
	sigc::signal<void> onChangeTarget;
	sigc::signal<void> onChangeFlightControlState;

private:
	struct InputBinding : public Input::InputFrame {
		using InputFrame::InputFrame;

		// Weapons
		Action *targetObject;
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
		Action *toggleSetSpeed;

		void RegisterBindings() override;
	} InputBindings;

	// FIXME: separate the propusion controller from the input system, pass in wanted velocity correction directly.
	friend class Propulsion;

	bool IsAnyAngularThrusterKeyDown();
	bool IsAnyLinearThrusterKeyDown();
	//do a variety of checks to see if input is allowed
	void CheckControlsLock();
	Body *m_combatTarget;
	Body *m_navTarget;
	Body *m_setSpeedTarget;
	bool m_controlsLocked;
	bool m_invertMouse; // used for rear view, *not* for invert Y-axis option (which is Pi::input->IsMouseYInvert)
	bool m_mouseActive;
	bool m_disableMouseFacing;
	bool m_rotationDamping;
	double m_mouseX;
	double m_mouseY;
	double m_setSpeed;
	FlightControlState m_flightControlState;
	float m_fovY; //for mouse acceleration adjustment
	float m_joystickDeadzone;
	float m_lowThrustPower;
	int m_combatTargetIndex; //for PostLoadFixUp
	int m_navTargetIndex;
	int m_setSpeedTargetIndex;
	vector3d m_mouseDir;

	sigc::connection m_connRotationDampingToggleKey;
	sigc::connection m_fireMissileKey;
	sigc::connection m_setSpeedMode;
};
