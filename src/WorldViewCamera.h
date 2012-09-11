#ifndef _WORLDVIEWCAMERA_H
#define _WORLDVIEWCAMERA_H
/*
 * Front, rear, external etc. cameras used by WorldView.
 */
#include "Camera.h"

class Ship;

class WorldViewCamera : public Camera
{
public:
	enum Type { //can be used for serialization & identification
		FRONT,
		REAR,
		EXTERNAL,
		SIDEREAL
	};

	//it is not strictly necessary, but WW cameras are now restricted to Ships
	WorldViewCamera(const Ship *s, const vector2f &size, float fovY, float nearClip, float farClip);
	virtual Type GetType() const = 0;
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
	virtual void Save(Serializer::Writer &wr) { }
	virtual void Load(Serializer::Reader &rd) { }
	virtual void Activate() { }
	virtual bool IsExternal() const { return false; }
};

// Forward facing view from the ship
class FrontCamera : public WorldViewCamera {
public:
	FrontCamera(const Ship *s, const vector2f &size, float fovY, float nearClip, float farClip);
	Type GetType() const { return FRONT; }
	void Activate();
};

// Rear-facing view
class RearCamera : public WorldViewCamera {
public:
	RearCamera(const Ship *s, const vector2f &size, float fovY, float nearClip, float farClip);
	Type GetType() const { return REAR; }
	void Activate();
};

// Zoomable, rotatable orbit camera, always looks at the ship
class ExternalCamera : public WorldViewCamera {
public:
	ExternalCamera(const Ship *s, const vector2f &size, float fovY, float nearClip, float farClip);
	Type GetType() const { return EXTERNAL; }

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
class SiderealCamera : public WorldViewCamera {
public:
	SiderealCamera(const Ship *s, const vector2f &size, float fovY, float nearClip, float farClip);
	Type GetType() const { return SIDEREAL; }
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
