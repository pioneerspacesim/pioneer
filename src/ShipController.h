#ifndef _SHIPCONTROLLER_H
#define _SHIPCONTROLLER_H
/*
 * Ship movement controller class
 * Controls thrusters, autopilot according to player input
 * Can be split into sub classes along the way, but now
 * this does "everything"
 */
#include "libs.h"
#include "Serializer.h"

class Ship;

enum FlightControlState {
	CONTROL_MANUAL,
	CONTROL_FIXSPEED,
	CONTROL_FIXHEADING_FORWARD,
	CONTROL_FIXHEADING_BACKWARD,
	CONTROL_AUTOPILOT,

	CONTROL_STATE_COUNT
};

// only AI
class ShipController
{
public:
	//needed for serialization
	enum Type {
		AI = 0,
		PLAYER = 1
	};
	ShipController() { }
	virtual ~ShipController() { }
	virtual Type GetType() { return AI; }
	virtual void Save(Serializer::Writer &wr) { }
	virtual void Load(Serializer::Reader &rd) { }
	virtual void StaticUpdate(float timeStep);
	virtual void SetFlightControlState(FlightControlState s) { }
	Ship *m_ship;
};

// autopilot AI + input
class PlayerShipController : public ShipController
{
public:
	PlayerShipController();
	~PlayerShipController();
	virtual Type GetType() { return PLAYER; }
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);

	void StaticUpdate(float timeStep);
	// Poll controls, set thruster states, gun states and target velocity
	void PollControls(float timeStep);
	bool IsMouseActive() const { return m_mouseActive; }
	double GetSetSpeed() const { return m_setSpeed; }
	FlightControlState GetFlightControlState() const { return m_flightControlState; }
	vector3d GetMouseDir() const { return m_mouseDir; }
	void SetMouseForRearView(bool enable) { m_invertMouse = enable; }
	void SetFlightControlState(FlightControlState s);

private:
	bool IsAnyAngularThrusterKeyDown();
	bool IsAnyLinearThrusterKeyDown();
	//do a variety of checks to see if input is allowed
	void CheckControlsLock();
	bool m_controlsLocked;
	bool m_invertMouse; // used for rear view, *not* for invert Y-axis option (which is Pi::IsMouseYInvert)
	bool m_mouseActive;
	double m_mouseX;
	double m_mouseY;
	double m_setSpeed;
	FlightControlState m_flightControlState;
	float m_fovY; //for mouse acceleration adjustment
	float m_joystickDeadzone;
	vector3d m_mouseDir;
};

#endif
