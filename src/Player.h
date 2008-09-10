#ifndef _PLAYER_H
#define _PLAYER_H

#include "libs.h"
#include "Ship.h"

class Player: public Ship {
public:
	Player(ShipType::Type shipType);
	virtual ~Player();
	void PollControls();
	virtual void Render(const Frame *camFrame);
	void DrawHUD(const Frame *cam_frame);
	virtual void SetDockedWith(SpaceStation *, int port);
	vector3d GetExternalViewTranslation();
	void ApplyExternalViewRotation(matrix4x4d &m);
	void TimeStepUpdate(const float timeStep);
private:
	void DrawTargetSquares();
	void DrawTargetSquare(const Body* const target);
	float m_mouseCMov[2];
	float m_external_view_rotx, m_external_view_roty;
	float m_external_view_dist;
	bool polledControlsThisTurn;
};

#endif /* _PLAYER_H */
