#ifndef _PLAYER_H
#define _PLAYER_H

#include "libs.h"
#include "Ship.h"

class Player: public Ship {
public:
	Player();
	virtual void AITurn();
	virtual void Render(const Frame *camFrame);
	void DrawHUD(const Frame *cam_frame);
	virtual void SetDockedWith(SpaceStation *);
	vector3d GetExternalViewTranslation();
	void ApplyExternalViewRotation();
private:
	float m_external_view_rotx, m_external_view_roty;
	float m_external_view_dist;
};

#endif /* _PLAYER_H */
