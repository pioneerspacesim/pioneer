#ifndef _PLAYER_H
#define _PLAYER_H

#include <list>
#include "libs.h"
#include "Ship.h"
#include "StarSystem.h"

class Player: public Ship {
public:
	OBJDEF(Player, Ship, PLAYER);
	Player(ShipType::Type shipType);
	Player() { m_mouseActive = false; }
	virtual ~Player();
	void PollControls(const float timeStep);
	virtual void Render(const vector3d &viewCoords, const matrix4x4d &viewTransform);
	virtual void SetDockedWith(SpaceStation *, int port);
	void StaticUpdate(const float timeStep);
	enum FlightControlState { CONTROL_MANUAL, CONTROL_FIXSPEED, CONTROL_AUTOPILOT };
	FlightControlState GetFlightControlState() const { return m_flightControlState; }
	void SetFlightControlState(FlightControlState s);
	float GetSetSpeed() const { return m_setSpeed; }
	virtual bool OnDamage(Object *attacker, float kgDamage);
	virtual void OnHaveKilled(Body *guyWeKilled);
	int GetKillCount() const { return m_knownKillCount; }
	virtual bool SetWheelState(bool down); // returns success of state change, NOT state itself

	// test code
	virtual void TimeStepUpdate(const float timeStep);
	vector3d GetAccumTorque() { return m_accumTorque; }
	vector3d m_accumTorque;
	vector3d GetMouseDir() { return m_mouseDir; }

double m_mouseAcc;
	
protected:
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
private:
//	float m_mouseCMov[2];
	vector3d m_mouseDir;
	bool m_mouseActive;
	bool polledControlsThisTurn;
	enum FlightControlState m_flightControlState;
	float m_setSpeed;
	int m_killCount;
	int m_knownKillCount; // updated on docking
};

#endif /* _PLAYER_H */
