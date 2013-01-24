// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "vector3.h"
#include "matrix4x4.h"
#include "Lang.h"
#include "Serializer.h"

class Ship;
class Camera;

class CameraController
{
public:
	enum Type { //can be used for serialization & identification
		INTERNAL,
		EXTERNAL,
		SIDEREAL
	};

	CameraController(Camera *camera, const Ship *ship);
	virtual ~CameraController() {}

	virtual Type GetType() const = 0;
	virtual const char *GetName() const { return ""; }
	virtual void Save(Serializer::Writer &wr) { }
	virtual void Load(Serializer::Reader &rd) { }
	virtual bool IsExternal() const { return false; }

	// camera position relative to the body
	void SetPosition(const vector3d &pos) { m_pos = pos; }
	vector3d GetPosition() const { return m_pos; }

	// camera orientation relative to the body
	void SetOrient(const matrix3x3d &orient) { m_orient = orient; }
	const matrix3x3d &GetOrient() const { return m_orient; }

	virtual void Update();

	const Ship *GetShip() const { return m_ship; }

private:
	Camera *m_camera;
	const Ship *m_ship;
	vector3d m_pos;
	matrix3x3d m_orient;
};

class InternalCameraController : public CameraController {
public:
	enum Mode {
		MODE_FRONT,
		MODE_REAR,
		MODE_LEFT,
		MODE_RIGHT,
		MODE_TOP,
		MODE_BOTTOM
	};

	InternalCameraController(Camera *camera, const Ship *ship);

	Type GetType() const { return INTERNAL; }
	const char *GetName() const { return m_name; }
	void SetMode(Mode m);
	Mode GetMode() const { return m_mode; }
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);

	virtual void Update();

private:
	Mode m_mode;
	const char *m_name;
};

class MoveableCameraController : public CameraController {
public:
	MoveableCameraController(Camera *camera, const Ship *ship) :
		CameraController(camera, ship) {}

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
};

// Zoomable, rotatable orbit camera, always looks at the ship
class ExternalCameraController : public MoveableCameraController {
public:
	ExternalCameraController(Camera *camera, const Ship *ship);

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
	bool IsExternal() const { return true; }
	void SetRotationAngles(double x, double y) {
		m_rotX = x;
		m_rotY = y;
	}

	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);

	void Update();

private:
	double m_dist, m_distTo;
	double m_rotX; //vertical rot
	double m_rotY; //horizontal rot
	matrix3x3d m_extOrient;
};

// Much like external camera, but does not turn when the ship turns
class SiderealCameraController : public MoveableCameraController {
public:
	SiderealCameraController(Camera *camera, const Ship *ship);

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
	bool IsExternal() const { return true; }

	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);

	void Update();

private:
	double m_dist, m_distTo;
	matrix3x3d m_sidOrient;
};

#endif
