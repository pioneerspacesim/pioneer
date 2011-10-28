#ifndef _PLAYER_H
#define _PLAYER_H

#include <list>
#include "libs.h"
#include "Ship.h"
#include "StarSystem.h"
#include "RefList.h"
#include "HyperspaceCloud.h"
#include "MarketAgent.h"

struct Mission : RefItem<Mission> {
	enum MissionState { // <enum scope='Mission' name=MissionStatus>
		ACTIVE,
		COMPLETED,
		FAILED,
	};

	std::string  type;
	std::string  client;
	SystemPath   location;
	double       due;
	Sint64       reward;
	MissionState status;
};

class Player: public Ship, public MarketAgent {
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
	double GetSetSpeed() const { return m_setSpeed; }
	virtual bool OnDamage(Object *attacker, float kgDamage);
	virtual void OnHaveKilled(Body *guyWeKilled);
	int GetKillCount() const { return m_knownKillCount; }
	virtual bool SetWheelState(bool down); // returns success of state change, NOT state itself
	virtual bool FireMissile(int idx, Ship *target);
	virtual void SetAlertState(Ship::AlertState as);
	bool IsAnyThrusterKeyDown();
	void SetNavTarget(Body* const target);
	Body *GetNavTarget() const { return m_navTarget; }
	void SetCombatTarget(Body* const target);
	Body *GetCombatTarget() const { return m_combatTarget; }
	virtual void NotifyDeleted(const Body* const deletedBody);

	// test code
	virtual void TimeStepUpdate(const float timeStep);
	vector3d GetAccumTorque() { return m_accumTorque; }
	vector3d m_accumTorque;
	vector3d GetMouseDir() { return m_mouseDir; }

	double m_mouseAcc;

	RefList<Mission> missions;

	virtual void PostLoadFixup();

	/* MarketAgent stuff */
	int GetStock(Equip::Type t) const { assert(0); return 0; }
	bool CanBuy(Equip::Type t, bool verbose) const;
	bool CanSell(Equip::Type t, bool verbose) const;
	bool DoesSell(Equip::Type t) const { return true; }
	Sint64 GetPrice(Equip::Type t) const;
protected:
	virtual void Save(Serializer::Writer &wr);
	virtual void Load(Serializer::Reader &rd);
	/* MarketAgent stuff */
	void Bought(Equip::Type t);
	void Sold(Equip::Type t);
private:
	vector3d m_mouseDir;
	double m_mouseX, m_mouseY;
	bool m_mouseActive;
	bool polledControlsThisTurn;
	enum FlightControlState m_flightControlState;
	double m_setSpeed;
	int m_killCount;
	int m_knownKillCount; // updated on docking
	Body* m_navTarget;
	Body* m_combatTarget;

	int m_combatTargetIndex, m_navTargetIndex; // deserialisation
};

#endif /* _PLAYER_H */
