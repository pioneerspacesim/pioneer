#pragma once
/*
 * Ship movement controller class
 * Controls thrusters, autopilot according to player input
 * Can be split into sub classes along the way, but now
 * this does "everything"
 */
#include "libs.h"

class Ship;

enum FlightControlState {
	CONTROL_MANUAL,
	CONTROL_FIXSPEED,
	CONTROL_FIXHEADING_FORWARD,
	CONTROL_FIXHEADING_BACKWARD,
	CONTROL_AUTOPILOT,

	CONTROL_STATE_COUNT
};

class ShipController
{
public:
	ShipController();
	~ShipController();
	void StaticUpdate(float timeStep);
	//disallow player input (autopilot OK)
	void LockControls();
	//allow player input
	void UnlockControls();
	// Poll controls, set thruster states, gun states and target velocity
	void PollControls(const float timeStep);
	//temporary, calls does lock/unlock constantly
	void CheckControlsLock();
	bool IsMouseActive() const { return m_mouseActive; }
	double GetSetSpeed() const { return m_setSpeed; }
	FlightControlState GetFlightControlState() const { return m_flightControlState; }
	Ship *m_ship;
	vector3d GetMouseDir() const { return m_mouseDir; }
	void SetFlightControlState(FlightControlState s);
	void SetMouseForRearView(bool enable) { m_invertMouse = enable; }

private:
	bool IsAnyAngularThrusterKeyDown();
	bool IsAnyLinearThrusterKeyDown();
	bool m_controlsLocked;
	bool m_invertMouse; // used for rear view, *not* for invert Y-axis option (which is Pi::IsMouseYInvert)
	bool m_mouseActive;
	double m_mouseX;
	double m_mouseY;
	double m_setSpeed;
	FlightControlState m_flightControlState;
	float m_joystickDeadzone;
	vector3d m_mouseDir;
};
