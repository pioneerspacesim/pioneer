// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H

#include "Camera.h"
#include "JsonFwd.h"
#include "Lang.h"
#include "Quaternion.h"
#include "matrix4x4.h"
#include "vector3.h"

class Ship;

class CameraController {
public:
	enum Type { //can be used for serialization & identification
		INTERNAL,
		EXTERNAL,
		SIDEREAL,
		FLYBY
	};

	CameraController(RefCountedPtr<CameraContext> camera, const Ship *ship);
	virtual ~CameraController() {}

	virtual void Reset();

	virtual Type GetType() const = 0;
	virtual const char *GetName() const { return ""; }
	virtual void SaveToJson(Json &jsonObj) {}
	virtual void LoadFromJson(const Json &jsonObj) {}
	virtual bool IsExternal() const { return false; }

	// camera position relative to the body
	void SetPosition(const vector3d &pos) { m_pos = pos; }
	vector3d GetPosition() const { return m_pos; }

	// camera orientation relative to the body
	void SetOrient(const matrix3x3d &orient) { m_orient = orient; }
	const matrix3x3d &GetOrient() const { return m_orient; }

	virtual void Update();
	virtual void OnActivated(){};
	virtual void OnDeactivated(){};

	const Ship *GetShip() const { return m_ship; }

protected:
	RefCountedPtr<CameraContext> m_camera;

private:
	const Ship *m_ship;
	vector3d m_pos;
	matrix3x3d m_orient;
};

class MoveableCameraController : public CameraController {
public:
	MoveableCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
		CameraController(camera, ship) {}

	// Rotate the camera in a specific axis.
	// `amount` is in radians and should be pre-multiplied by the frame delta.
	virtual void RollCamera(float amount) {}
	virtual void PitchCamera(float amount) {}
	virtual void YawCamera(float amount) {}

	// Manually set the camera's rotation angle.
	virtual void SetRotationAngles(vector3f rotation) {}

	/// Animated zoom trigger (on each event), primarily designed for mouse wheel.
	///\param amount The zoom delta to add or substract (>0: zoom out, <0: zoom in), indirectly controling the zoom animation speed.
	virtual void ZoomEvent(float amount) {}
	/// Animated zoom update (on each frame), primarily designed for mouse wheel.
	virtual void ZoomEventUpdate(float frameTime) {}
};

class InternalCameraController : public MoveableCameraController {
public:
	enum Mode {
		MODE_FRONT = 0,
		MODE_REAR,
		MODE_LEFT,
		MODE_RIGHT,
		MODE_TOP,
		MODE_BOTTOM,
		MODE_MAX
	};

	InternalCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship);
	void Reset() override;
	void Update() override;

	Type GetType() const override { return INTERNAL; }
	const char *GetName() const override { return m_name; }

	void SetSmoothingEnabled(bool enabled);
	bool GetSmoothingEnabled() const { return m_smoothing; }

	void SetMode(Mode m);
	Mode GetMode() const { return m_mode; }

	void ZoomEvent(float amount) override;
	void ZoomEventUpdate(float frameTime) override;

	void SaveToJson(Json &jsonObj) override;
	void LoadFromJson(const Json &jsonObj) override;

	void OnDeactivated() override;

	void PitchCamera(float amount) override { m_rotToX += amount * m_zoomPct; }
	void YawCamera(float amount) override { m_rotToY += amount * m_zoomPct; }

	void SetRotationAngles(vector3f rotation) override
	{
		m_rotToX = rotation.x;
		m_rotToY = rotation.y;
	}

	// TODO: remove this and replace with a better function.
	void getRots(double &rotX, double &rotY);

private:
	Mode m_mode;
	const char *m_name;

	matrix3x3d m_initOrient[MODE_MAX];
	vector3d m_initPos[MODE_MAX];

	float m_rotToX; //vertical target rot
	float m_rotToY; //horizontal target rot
	float m_rotX;	//vertical rot
	float m_rotY;	//horizontal rot
	float m_origFov;
	float m_zoomPct;
	float m_zoomPctTo;
	matrix3x3d m_viewOrient;

	bool m_smoothing;
};

// Zoomable, rotatable orbit camera, always looks at the ship
class ExternalCameraController : public MoveableCameraController {
public:
	ExternalCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship);

	Type GetType() const override { return EXTERNAL; }
	const char *GetName() const override { return Lang::EXTERNAL_VIEW; }
	bool IsExternal() const override { return true; }

	virtual void PitchCamera(float amount) override { m_rotX += amount; }
	virtual void YawCamera(float amount) override { m_rotY += amount; }

	virtual void SetRotationAngles(vector3f rotation) override
	{
		m_rotX = rotation.x;
		m_rotY = rotation.y;
	}

	void ZoomEvent(float amount) override;
	void ZoomEventUpdate(float frameTime) override;

	void SaveToJson(Json &jsonObj) override;
	void LoadFromJson(const Json &jsonObj) override;

	void Update() override;
	void Reset() override;

private:
	double m_dist, m_distTo;
	double m_rotX; //vertical rot
	double m_rotY; //horizontal rot
	matrix3x3d m_extOrient;
	Quaternionf m_smoothed_ship_orient;
};

// Much like external camera, but does not turn when the ship turns
class SiderealCameraController : public MoveableCameraController {
public:
	SiderealCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship);

	Type GetType() const override { return SIDEREAL; }
	const char *GetName() const override { return Lang::SIDEREAL_VIEW; }
	bool IsExternal() const override { return true; }

	void PitchCamera(float amount) override
	{
		const vector3d rotAxis = m_sidOrient.VectorX();
		m_sidOrient = matrix3x3d::Rotate(M_PI / 4 * amount, rotAxis) * m_sidOrient;
	}

	void YawCamera(float amount) override
	{
		const vector3d rotAxis = m_sidOrient.VectorY();
		m_sidOrient = matrix3x3d::Rotate(M_PI / 4 * amount, rotAxis) * m_sidOrient;
	}

	void RollCamera(float amount) override
	{
		const vector3d rotAxis = m_sidOrient.VectorZ();
		m_sidOrient = matrix3x3d::Rotate(M_PI / 4 * amount, rotAxis) * m_sidOrient;
	}

	// Apply in YXZ order because euler angles are non-ideal.
	void SetRotationAngles(vector3f rotation) override
	{
		m_sidOrient = matrix3x3d::Identity();
		YawCamera(rotation.y);
		PitchCamera(rotation.x);
		RollCamera(rotation.z);
	}

	void ZoomEvent(float amount) override;
	void ZoomEventUpdate(float frameTime) override;

	void SaveToJson(Json &jsonObj) override;
	void LoadFromJson(const Json &jsonObj) override;

	void Update() override;
	void Reset() override;

private:
	double m_dist, m_distTo;
	matrix3x3d m_sidOrient;
};

// Zoomable, fly by camera, always looks at the ship
class FlyByCameraController : public MoveableCameraController {
public:
	FlyByCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship);

	Type GetType() const override { return FLYBY; }
	const char *GetName() const override { return Lang::FLYBY_VIEW; }
	bool IsExternal() const override { return true; }

	void PitchCamera(float amount) override
	{
		const vector3d rotAxis = m_flybyOrient.VectorX();
		m_flybyOrient = matrix3x3d::Rotate(M_PI / 4 * amount, rotAxis) * m_flybyOrient;
	}

	void YawCamera(float amount) override
	{
		const vector3d rotAxis = m_flybyOrient.VectorY();
		m_flybyOrient = matrix3x3d::Rotate(M_PI / 4 * amount, rotAxis) * m_flybyOrient;
	}

	void RollCamera(float amount) override
	{
		m_roll += M_PI / 4 * amount;
	}

	void ZoomEvent(float amount) override;
	void ZoomEventUpdate(float frameTime) override;

	void SaveToJson(Json &jsonObj) override;
	void LoadFromJson(const Json &jsonObj) override;

	void Update() override;
	void Reset() override;

private:
	double m_dist, m_distTo;
	float m_roll;
	matrix3x3d m_flybyOrient;
	vector3d m_old_pos;
	Frame *m_old_frame;
};

#endif
