#ifndef _PLAYER_H
#define _PLAYER_H

#include <list>
#include "libs.h"
#include "Ship.h"
#include "StarSystem.h"

class Mission;

class Player: public Ship {
public:
	OBJDEF(Player, Ship, PLAYER);
	Player(ShipType::Type shipType);
	Player() {}
	virtual ~Player();
	void PollControls();
	virtual void Render(const Frame *camFrame);
	virtual void SetDockedWith(SpaceStation *, int port);
	void StaticUpdate(const float timeStep);
	enum FlightControlState { CONTROL_MANUAL, CONTROL_FIXSPEED, CONTROL_AUTOPILOT };
	FlightControlState GetFlightControlState() const { return m_flightControlState; }
	void SetFlightControlState(FlightControlState s);
	float GetSetSpeed() const { return m_setSpeed; }
	const SBodyPath *GetHyperspaceTarget() const { return &m_hyperspaceTarget; }
	void SetHyperspaceTarget(const SBodyPath *path);
	void TakeMission(Mission *);
	const std::list<Mission*> &GetMissions() const { return m_missions; }
	virtual bool OnDamage(Object *attacker, float kgDamage);
	virtual void OnHaveKilled(Body *guyWeKilled);
	int GetKillCount() const { return m_knownKillCount; }
protected:
	virtual void Save();
	virtual void Load();
private:
	std::list<Mission*> m_missions;
	float m_mouseCMov[2];
	bool polledControlsThisTurn;
	enum FlightControlState m_flightControlState;
	float m_setSpeed;
	int m_killCount;
	int m_knownKillCount; // updated on docking
	SBodyPath m_hyperspaceTarget;
};

#endif /* _PLAYER_H */
