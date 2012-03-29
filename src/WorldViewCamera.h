﻿#ifndef _WORLDVIEWCAMERA_H
#define _WORLDVIEWCAMERA_H
/*
 * Front, rear, external etc. cameras used by WorldView.
 */
#include "Camera.h"

class WorldViewCamera : public Camera
{
public:
	enum Type { //can be used for serialization & identification
		FRONT,
		REAR,
		EXTERNAL,
		SIDEREAL
	};

	WorldViewCamera(const Body *b, const vector2f &size, float fovY, float nearClip, float farClip);
	virtual Type GetType() const = 0;
	virtual void RollLeft(float frameTime) { }
	virtual void RollRight(float frameTime) { }
	virtual void RotateDown(float frameTime) { }
	virtual void RotateLeft(float frameTime) { }
	virtual void RotateRight(float frameTime) { }
	virtual void RotateUp(float frameTime) { }
	virtual void ZoomIn(float frameTime) { }
	virtual void ZoomOut(float frameTime) { }
	virtual void Reset() { }
	//set translation & orientation
	virtual void UpdateTransform() { }
	virtual void Save(Serializer::Writer &wr) { }
	virtual void Load(Serializer::Reader &rd) { }
};

// Forward facing view from the ship
class FrontCamera : public WorldViewCamera {
public:
	FrontCamera(const Body *b, const vector2f &size, float fovY, float nearClip, float farClip) :
	  WorldViewCamera(b, size, fovY, nearClip, farClip) { }
	Type GetType() const { return FRONT; }
};

// Rear-facing view
class RearCamera : public WorldViewCamera {
public:
	RearCamera(const Body *b, const vector2f &size, float fovY, float nearClip, float farClip);
	Type GetType() const { return REAR; }
};

// Zoomable, rotatable orbit camera, always looks at the ship
class ExternalCamera : public WorldViewCamera {
public:
	ExternalCamera(const Body *b, const vector2f &size, float fovY, float nearClip, float farClip);
	Type GetType() const { return EXTERNAL; }

	void RotateDown(float frameTime);
	void RotateLeft(float frameTime);
	void RotateRight(float frameTime);
	void RotateUp(float frameTime);
	void ZoomIn(float frameTime);
	void ZoomOut(float frameTime);
	void Reset();
	void UpdateTransform();
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);
	void SetRotationAngles(double x, double y) {
		m_rotX = x;
		m_rotY = y;
	}
private:
	double m_dist;
	double m_rotX; //vertical rot
	double m_rotY; //horizontal rot
	matrix4x4d m_orient;
};

// Much like external camera, but does not turn when the ship turns
class SiderealCamera : public WorldViewCamera {
public:
	SiderealCamera(const Ship *b, const vector2f &size, float fovY, float nearClip, float farClip);
	Type GetType() const { return SIDEREAL; }
	void RollLeft(float frameTime);
	void RollRight(float frameTime);
	void RotateDown(float frameTime);
	void RotateLeft(float frameTime);
	void RotateRight(float frameTime);
	void RotateUp(float frameTime);
	void ZoomIn(float frameTime);
	void ZoomOut(float frameTime);
	void Reset();
	void UpdateTransform();
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);
private:
	double m_dist;
	matrix4x4d m_orient;
	matrix4x4d m_prevShipOrient;
};

#endif