// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SHIP_COCKPIT_H_
#define _SHIP_COCKPIT_H_

#include "libs.h"
#include "ModelBody.h"

struct ShipType;

static const float COCKPIT_LAG_MAX_ANGLE = 7.5f;
static const float COCKPIT_ROTATION_INTERP_MULTIPLIER = 5.0f;
static const float COCKPIT_ACCEL_INTERP_MULTIPLIER = 0.5f;
static const float COCKPIT_MAX_GFORCE = 10000.0f;
static const float COCKPIT_ACCEL_OFFSET = 0.075f;

enum CockpitLagEasing
{
	CLE_LINEAR_EASING,
	CLE_CUBIC_EASING,
	CLE_QUAD_EASING,
	CLE_EXP_EASING,
};

class ShipCockpit : public ModelBody
{
public:
	explicit ShipCockpit(const ShipType& ship_type);
	virtual ~ShipCockpit();

	virtual void Render(Graphics::Renderer *r, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform) override;
	
	void Update(float timeStep);
	void RenderCockpit(Graphics::Renderer* renderer, const Camera* camera, const Frame* frame);
	void OnActivated();

protected:
	void Init();
	float CalculateSignedForwardVelocity(vector3d forward, vector3d velocity);
	float EaseOut(float a, float b, float delta);
	float EaseIn(float a, float b, float delta);

private:
	ShipCockpit(const ShipCockpit&);
	ShipCockpit& operator=(const ShipCockpit&);
	
	ShipType m_type;

	vector3d vShipDir;		// current ship direction
	vector3d vShipYaw;		// current ship yaw vector
	vector3d vdDir;			// cockpit direction
	vector3d vdYaw;			// cockpit yaw vector
	float fRInterp;			// for rotation interpolation
	float fTInterp;			// for translation interpolation
	float fGForce;			// current ship gforce
	float fOffset;			// current ship offset due to acceleration effect
	float fShipVel;			// current ship velocity
	CockpitLagEasing eEasing; // Easing function for lag recover
	vector3d vTranslate;    // cockpit translation
	matrix4x4d matTransform;// cockpit transformation
};

#endif