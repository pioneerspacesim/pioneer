// Copyright © 2013-14 Meteoric Games Ltd
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIP_COCKPIT_H_
#define _SHIP_COCKPIT_H_

#include "ModelBody.h"

static const float COCKPIT_LAG_MAX_ANGLE = 7.5f;
static const float COCKPIT_ROTATION_INTERP_MULTIPLIER = 5.0f;
static const float COCKPIT_ACCEL_INTERP_MULTIPLIER = 0.5f;
static const float COCKPIT_MAX_GFORCE = 10000.0f;
static const float COCKPIT_ACCEL_OFFSET = 0.075f;

class Player;
class Camera;
class InternalCameraController;

class ShipCockpit : public ModelBody {
public:
	explicit ShipCockpit(const std::string &modelName, Body *ship);
	virtual ~ShipCockpit();

	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;

	void Update(const Player *player, float timeStep);
	void RenderCockpit(Graphics::Renderer *renderer, const Camera *camera, FrameId frameId);
	void OnActivated(const Player *player);
	void resetInternalCameraController(void);

protected:
	float CalculateSignedForwardVelocity(const vector3d &forward, const vector3d &velocity);

private:
	ShipCockpit(const ShipCockpit &) = delete;
	ShipCockpit &operator=(const ShipCockpit &) = delete;

	Body *m_ship; // current ship body, for rendering

	vector3d m_shipDir; // current ship direction
	vector3d m_shipYaw; // current ship yaw vector
	vector3d m_dir; // cockpit direction
	vector3d m_yaw; // cockpit yaw vector
	float m_rotInterp; // for rotation interpolation
	float m_transInterp; // for translation interpolation
	float m_gForce; // current ship gforce
	float m_offset; // current ship offset due to acceleration effect
	float m_shipVel; // current ship velocity
	vector3d m_translate; // cockpit translation
	matrix4x4d m_transform; // cockpit transformation
	InternalCameraController *m_icc;
};
#endif
