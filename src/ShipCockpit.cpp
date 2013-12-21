// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipCockpit.h"
#include "ShipType.h"
#include "Pi.h"
#include "Player.h"
#include "MathUtil.h"

ShipCockpit::ShipCockpit(const ShipType& ship_type) :
	m_type(ship_type), matTransform(matrix4x4d::Identity()), vTranslate(vector3d(0.0, 0.0, 0.0)),
	fRInterp(0.0f), fTInterp(0.0f), fGForce(0.0f), fOffset(0.0f), eEasing(CLE_QUAD_EASING)
{
	Init();
}

ShipCockpit::~ShipCockpit()
{

}

void ShipCockpit::Init()
{
	assert(!m_type.cockpitName.empty());
	SetModel(m_type.cockpitName.c_str());
	SetColliding(false);
	assert(GetModel());
}

void ShipCockpit::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	RenderModel(renderer, camera, viewCoords, viewTransform);
}

void ShipCockpit::Update(float timeStep)
{
	matTransform = matrix4x4d::Identity();
	vector3d cur_dir = Pi::player->GetOrient().VectorZ().Normalized();
	if(cur_dir.Dot(vShipDir) < 1.0f) {
		fRInterp = 0.0f;
		vShipDir = cur_dir;
	}

	//---------------------------------------- Acceleration
	float cur_vel = CalculateSignedForwardVelocity(-cur_dir, Pi::player->GetVelocity()); // Forward is -Z
	float gforce = Clamp(floorf(((abs(cur_vel) - fShipVel) / timeStep) / 9.8f), -COCKPIT_MAX_GFORCE, COCKPIT_MAX_GFORCE);
	if(abs(cur_vel) > 500000.0f ||      // Limit gforce measurement so we don't get astronomical fluctuations
	   abs(gforce - fGForce) > 100.0) { // Smooth out gforce one frame spikes, sometimes happens when hitting max speed due to the thrust limiters
		gforce = 0.0f;
	}
	if(abs(gforce - fGForce) > 100.0) {
		gforce = 0.0f;
	}
	if(abs(vTranslate.z - fOffset) < 0.001f) {
		fTInterp = 0.0f;
	}
	float offset = (gforce > 14.0f? -1.0f : (gforce < -14.0f? 1.0f : 0.0f)) * COCKPIT_ACCEL_OFFSET;
	fTInterp += timeStep * COCKPIT_ACCEL_INTERP_MULTIPLIER;
	if(fTInterp > 1.0f) {
		fTInterp = 1.0f;	
		vTranslate.z = offset;
	}
	vTranslate.z = EaseIn(vTranslate.z, offset, fTInterp);
	fGForce = gforce;
	fOffset = offset;
	fShipVel = cur_vel;

	//------------------------------------------ Rotation
	// For yaw/pitch
	vector3d rot_axis = cur_dir.Cross(vdDir).Normalized();
	vector3d yaw_axis = Pi::player->GetOrient().VectorY().Normalized();
	vector3d pitch_axis = Pi::player->GetOrient().VectorX().Normalized();
	float dot = cur_dir.Dot(vdDir);
	float angle = acos(dot);
	// For roll
	if(yaw_axis.Dot(vShipYaw) < 1.0f) {
		fRInterp = 0.0f;
		vShipYaw = yaw_axis;
	}
	vector3d rot_yaw_axis = yaw_axis.Cross(vdYaw).Normalized();
	float dot_yaw = yaw_axis.Dot(vdYaw);
	float angle_yaw = acos(dot_yaw);

	if(dot < 1.0f || dot_yaw < 1.0f) {
		// Lag/Recovery interpolation
		fRInterp += timeStep * COCKPIT_ROTATION_INTERP_MULTIPLIER;
		if(fRInterp > 1.0f) {
			fRInterp = 1.0f;
		}

		// Yaw and Pitch
		if(dot < 1.0f) {
			if(angle > DEG2RAD(COCKPIT_LAG_MAX_ANGLE)) {
				angle = DEG2RAD(COCKPIT_LAG_MAX_ANGLE);
			}
			angle = EaseOut(angle, 0.0, fRInterp);
			vdDir = cur_dir;
			if(angle >= 0.0f) {
				vdDir.ArbRotate(rot_axis, angle);
				// Apply pitch
				vector3d yz_proj = (vdDir - (vdDir.Dot(pitch_axis) * pitch_axis)).Normalized();
				float pitch_cos = yz_proj.Dot(cur_dir);
				float pitch_angle = 0.0f;
				if(pitch_cos < 1.0f) {
					pitch_angle = acos(pitch_cos);
					if(rot_axis.Dot(pitch_axis) < 0) {
						pitch_angle = -pitch_angle;
					}
					matTransform.RotateX(-pitch_angle);
				}
				// Apply yaw
				vector3d xz_proj = (vdDir - (vdDir.Dot(yaw_axis) * yaw_axis)).Normalized();
				float yaw_cos = xz_proj.Dot(cur_dir);
				float yaw_angle = 0.0f;
				if(yaw_cos < 1.0f) {
					yaw_angle = acos(yaw_cos);
					if(rot_axis.Dot(yaw_axis) < 0) {
						yaw_angle = -yaw_angle;
					}
					matTransform.RotateY(-yaw_angle);
				}
			} else {
				angle = 0.0f;
			}
		}

		// Roll
		if(dot_yaw < 1.0f) {			
			if(angle_yaw > DEG2RAD(COCKPIT_LAG_MAX_ANGLE)) {
				angle_yaw = DEG2RAD(COCKPIT_LAG_MAX_ANGLE);
			}
			if(dot_yaw < 1.0f) {
				angle_yaw = EaseOut(angle_yaw, 0.0, fRInterp);
			}
			vdYaw = yaw_axis;
			if(angle_yaw >= 0.0f) {
				vdYaw.ArbRotate(rot_yaw_axis, angle_yaw);
				// Apply roll
				vector3d xy_proj = (vdYaw - (vdYaw.Dot(cur_dir) * cur_dir)).Normalized();
				float roll_cos = xy_proj.Dot(yaw_axis);
				float roll_angle = 0.0f;
				if(roll_cos < 1.0f) {
					roll_angle = acos(roll_cos);
					if(rot_yaw_axis.Dot(cur_dir) < 0) {
						roll_angle = -roll_angle;
					}
					matTransform.RotateZ(-roll_angle);
				}
			} else {
				angle_yaw = 0.0f;
			}
		}
	} else {
		fRInterp = 0.0f;
	}
}

void ShipCockpit::RenderCockpit(Graphics::Renderer* renderer, const Camera* camera, const Frame* frame)
{
	renderer->ClearDepthBuffer();
	SetFrame(const_cast<Frame*>(frame));
	Render(renderer, camera, vTranslate, matTransform);
	SetFrame(nullptr);
}

void ShipCockpit::OnActivated()
{
	assert(Pi::player);
	vdDir = Pi::player->GetOrient().VectorZ().Normalized();
	vdYaw = Pi::player->GetOrient().VectorY().Normalized();
	vShipDir = vdDir;
	vShipYaw = vdYaw;
	fShipVel = CalculateSignedForwardVelocity(-vShipDir, Pi::player->GetVelocity());
}

float ShipCockpit::CalculateSignedForwardVelocity(vector3d normalized_forward, vector3d velocity) {
	float velz_cos = velocity.Dot(normalized_forward);
	return (velz_cos * normalized_forward).Length() * (velz_cos < 0.0? -1.0 : 1.0);
}

float ShipCockpit::EaseOut(float a, float b, float delta)
{
	switch(eEasing) {
		case CLE_CUBIC_EASING:
			return MathUtil::CubicInterpOut<float>(a, b, delta);
			break;

		case CLE_QUAD_EASING:
			return MathUtil::QuadInterpOut<float>(a, b, delta);
			break;

		case CLE_EXP_EASING:
			return MathUtil::ExpInterpOut<float>(a, b, delta);
			break;

		default:
			return MathUtil::LinearInterp<float>(a, b, delta);
	}
}

float ShipCockpit::EaseIn(float a, float b, float delta)
{
	switch(eEasing) {
		case CLE_CUBIC_EASING:
			return MathUtil::CubicInterpIn<float>(a, b, delta);
			break;

		case CLE_QUAD_EASING:
			return MathUtil::QuadInterpIn<float>(a, b, delta);
			break;

		case CLE_EXP_EASING:
			return MathUtil::ExpInterpIn<float>(a, b, delta);
			break;

		default:
			return MathUtil::LinearInterp<float>(a, b, delta);
	}
}