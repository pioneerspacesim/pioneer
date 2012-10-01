// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _WORLDVIEWCAMERA_H
#define _WORLDVIEWCAMERA_H
/*
 * Front, rear, external etc. cameras used by WorldView.
 */
#include "Camera.h"
#include "Lang.h"

class Ship;

class WorldViewCamera : public Camera
{
public:
	enum Type { //can be used for serialization & identification
		INTERNAL,
		EXTERNAL,
		SIDEREAL
	};

	//it is not strictly necessary, but WW cameras are now restricted to Ships
	WorldViewCamera(const Ship *s, const vector2f &size, float fovY, float nearClip, float farClip);
	virtual Type GetType() const = 0;
	virtual const char *GetName() const { return ""; }
	virtual void Save(Serializer::Writer &wr) { }
	virtual void Load(Serializer::Reader &rd) { }
	virtual void Activate() { }
	virtual bool IsExternal() const { return false; }
};

// Front view from the cockpit.
class InternalCamera : public WorldViewCamera {
public:
	InternalCamera(const Ship *s, const vector2f &size, float fovY, float nearClip, float farClip);
	Type GetType() const { return INTERNAL; }
	const char *GetName() const { return m_name; }
	void FrontCockpit();
	void RearCockpit();
	void Front();
	void Rear();
	void Left();
	void Right();
	void Top();
	void Bottom();
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);
	void Activate();
private:
	matrix4x4d m_orient;
	vector3d m_offs;
	const char *m_name;
};

class MoveableCamera : public WorldViewCamera {
public:
	MoveableCamera(const Ship *s, const vector2f &size, float fovY, float nearClip, float farClip) :
		WorldViewCamera(s, size, fovY, nearClip, farClip) {}
	virtual void RollLeft(float frameTime) { }
	virtual void RollRight(float frameTime) { }
	virtual void RotateDown(float frameTime) { }
	virtual void RotateLeft(float frameTime) { }
	virtual void RotateRight(float frameTime) { }
	virtual void RotateUp(float frameTime) { }
	/// Zooming with this method will interrupt any animation launched by ZoomEvent().
	virtual void ZoomIn(float frameTime) { }
	/// Zooming with this method will interrupt any animation launched by ZoomEvent().
	virtual void ZoomOut(float frameTime) { }
	/// Animated zoom trigger (on each event), primarily designed for mouse wheel.
	///\param amount The zoom delta to add or substract (>0: zoom out, <0: zoom in), indirectly controling the zoom animation speed.
	virtual void ZoomEvent(float amount) { }
	/// Animated zoom update (on each frame), primarily designed for mouse wheel.
	virtual void ZoomEventUpdate(float frameTime) { }
	virtual void Reset() { }
	//set translation & orientation
	virtual void UpdateTransform() { }
};

// Zoomable, rotatable orbit camera, always looks at the ship
class ExternalCamera : public MoveableCamera {
public:
	ExternalCamera(const Ship *s, const vector2f &size, float fovY, float nearClip, float farClip);
	Type GetType() const { return EXTERNAL; }
	const char *GetName() const { return Lang::EXTERNAL_VIEW; }

	void RotateDown(float frameTime);
	void RotateLeft(float frameTime);
	void RotateRight(float frameTime);
	void RotateUp(float frameTime);
	void ZoomIn(float frameTime);
	void ZoomOut(float frameTime);
	void ZoomEvent(float amount);
	void ZoomEventUpdate(float frameTime);
	void Reset();
	void UpdateTransform();
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);
	bool IsExternal() const { return true; }
	void SetRotationAngles(double x, double y) {
		m_rotX = x;
		m_rotY = y;
	}
private:
	double m_dist, m_distTo;
	double m_rotX; //vertical rot
	double m_rotY; //horizontal rot
	matrix4x4d m_orient;
};

// Much like external camera, but does not turn when the ship turns
class SiderealCamera : public MoveableCamera {
public:
	SiderealCamera(const Ship *s, const vector2f &size, float fovY, float nearClip, float farClip);
	Type GetType() const { return SIDEREAL; }
	const char *GetName() const { return Lang::SIDEREAL_VIEW; }

	void RollLeft(float frameTime);
	void RollRight(float frameTime);
	void RotateDown(float frameTime);
	void RotateLeft(float frameTime);
	void RotateRight(float frameTime);
	void RotateUp(float frameTime);
	void ZoomIn(float frameTime);
	void ZoomOut(float frameTime);
	void ZoomEvent(float amount);
	void ZoomEventUpdate(float frameTime);
	void Reset();
	void UpdateTransform();
	bool IsExternal() const { return true; }
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);
private:
	double m_dist, m_distTo;
	matrix4x4d m_orient;
	matrix4x4d m_prevShipOrient;
};

#endif
