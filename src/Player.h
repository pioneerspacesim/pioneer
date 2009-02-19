#ifndef _PLAYER_H
#define _PLAYER_H

#include "libs.h"
#include "Ship.h"
#include "StarSystem.h"

class Player: public Ship {
public:
	OBJDEF(Player, Ship, PLAYER);
	Player(ShipType::Type shipType);
	Player() {}
	virtual ~Player();
	void PollControls();
	virtual void Render(const Frame *camFrame);
	void DrawHUD(const Frame *cam_frame);
	virtual void SetDockedWith(SpaceStation *, int port);
	void TimeStepUpdate(const float timeStep);
	enum FlightControlState { CONTROL_MANUAL, CONTROL_FIXSPEED, CONTROL_AUTOPILOT };
	FlightControlState GetFlightControlState() const { return m_flightControlState; }
	void SetFlightControlState(FlightControlState s);
	const SBodyPath *GetHyperspaceTarget() const { return &m_hyperspaceTarget; }
	void SetHyperspaceTarget(const SBodyPath *path);
protected:
	virtual void Save();
	virtual void Load();
private:
	void DrawTargetSquares();
	void DrawTargetSquare(const Body* const target);
	float m_mouseCMov[2];
	bool polledControlsThisTurn;
	enum FlightControlState m_flightControlState;
	float m_setSpeed;
	SBodyPath m_hyperspaceTarget;
};

#endif /* _PLAYER_H */
