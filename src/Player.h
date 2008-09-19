#ifndef _PLAYER_H
#define _PLAYER_H

#include "libs.h"
#include "Ship.h"

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
private:
	void DrawTargetSquares();
	void DrawTargetSquare(const Body* const target);
	float m_mouseCMov[2];
	bool polledControlsThisTurn;
};

#endif /* _PLAYER_H */
