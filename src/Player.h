#ifndef _PLAYER_H
#define _PLAYER_H

#include "libs.h"
#include "Ship.h"

class Player: public Ship {
public:
	Player(ShipType::Type shipType);
	virtual void AITurn();
	virtual void Render(const Frame *camFrame);
	void DrawHUD(const Frame *cam_frame);
	virtual void SetDockedWith(SpaceStation *);
	vector3d GetExternalViewTranslation();
	void ApplyExternalViewRotation();
private:
	void DrawTargetSquare();
	float m_mouseCMov[2];
	float m_external_view_rotx, m_external_view_roty;
	float m_external_view_dist;
};

#endif /* _PLAYER_H */
