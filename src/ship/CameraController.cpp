// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CameraController.h"

#include "AnimationCurves.h"
#include "Frame.h"
#include "Game.h"
#include "GameSaveError.h"
#include "JsonUtils.h"
#include "MathUtil.h"
#include "Pi.h"
#include "Quaternion.h"
#include "Ship.h"
#include "Space.h"
#include "collider/CollisionContact.h"
#include "collider/CollisionSpace.h"
#include "scenegraph/Tag.h"

CameraController::CameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	m_camera(camera),
	m_ship(ship),
	m_pos(0.0),
	m_orient(matrix3x3d::Identity())
{
}

void CameraController::Reset()
{
	m_pos = vector3d(0.0);
	m_orient = matrix3x3d::Identity();
}

void CameraController::Update()
{
	m_camera->SetCameraFrame(m_ship->GetFrame());
	if (GetType() == FLYBY) {
		m_camera->SetCameraOrient(m_orient);
		m_camera->SetCameraPosition(m_pos);
	} else {
		const matrix3x3d &m = m_ship->GetInterpOrient();

		// interpolate between last physics tick position and current one,
		// to remove temporal aliasing
		m_camera->SetCameraOrient(m * m_orient);
		m_camera->SetCameraPosition(m * m_pos + m_ship->GetInterpPosition());
	}
}

InternalCameraController::InternalCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	MoveableCameraController(camera, ship),
	m_mode(MODE_FRONT),
	m_rotToX(0),
	m_rotToY(0),
	m_rotX(0),
	m_rotY(0),
	m_origFov(camera->GetFovAng()),
	m_zoomPct(1),
	m_zoomPctTo(1),
	m_viewOrient(matrix3x3d::Identity()),
	m_smoothing(false)
{
	Reset();
}

static bool FillCameraPosOrient(const SceneGraph::Model *m, const char *tag, vector3d &pos, matrix3x3d &orient, matrix4x4f &trans, const matrix3x3d &fallbackOrient)
{
	matrix3x3d fixOrient(matrix3x3d::Identity());

	const SceneGraph::Tag *tagNode = m->FindTagByName(tag);
	if (!tagNode) {
		fixOrient = fallbackOrient;
	} else {
		// camera points are have +Z pointing out of the ship, X left, so we
		// have to rotate 180 about Y to get them to -Z forward, X right like
		// the rest of the ship. this is not a bug, but rather a convenience to
		// modellers. it makes sense to orient the camera point in the
		// direction the camera will face
		trans = tagNode->GetGlobalTransform() * matrix4x4f::RotateYMatrix(M_PI);
	}

	pos = vector3d(trans.GetTranslate());
	orient = fixOrient * matrix3x3d(trans.GetOrient());

	return true;
}

void InternalCameraController::Reset()
{
	CameraController::Reset();

	const SceneGraph::Model *m = GetShip()->GetModel();

	matrix4x4f fallbackTransform = matrix4x4f::Translation(vector3f(0.0));
	const SceneGraph::Tag *fallback = m->FindTagByName("tag_camera");
	if (fallback)
		fallbackTransform = fallback->GetGlobalTransform() * matrix4x4f::RotateYMatrix(M_PI);

	FillCameraPosOrient(m, "tag_camera_front", m_initPos[MODE_FRONT], m_initOrient[MODE_FRONT], fallbackTransform, matrix3x3d::Identity());
	FillCameraPosOrient(m, "tag_camera_rear", m_initPos[MODE_REAR], m_initOrient[MODE_REAR], fallbackTransform, matrix3x3d::RotateY(M_PI));
	FillCameraPosOrient(m, "tag_camera_left", m_initPos[MODE_LEFT], m_initOrient[MODE_LEFT], fallbackTransform, matrix3x3d::RotateY((M_PI / 2) * 3));
	FillCameraPosOrient(m, "tag_camera_right", m_initPos[MODE_RIGHT], m_initOrient[MODE_RIGHT], fallbackTransform, matrix3x3d::RotateY(M_PI / 2));
	FillCameraPosOrient(m, "tag_camera_top", m_initPos[MODE_TOP], m_initOrient[MODE_TOP], fallbackTransform, matrix3x3d::RotateX((M_PI / 2) * 3));
	FillCameraPosOrient(m, "tag_camera_bottom", m_initPos[MODE_BOTTOM], m_initOrient[MODE_BOTTOM], fallbackTransform, matrix3x3d::RotateX(M_PI / 2));

	SetMode(m_mode);
}

void InternalCameraController::Update()
{
	if (m_smoothing) {
		AnimationCurves::Approach(m_rotX, m_rotToX, Pi::GetFrameTime(), 13.f);
		AnimationCurves::Approach(m_rotY, m_rotToY, Pi::GetFrameTime(), 13.f);
	} else {
		m_rotX = m_rotToX;
		m_rotY = m_rotToY;
	}

	m_viewOrient =
		matrix3x3d::RotateY(-m_rotY) *
		matrix3x3d::RotateX(-m_rotX);

	SetOrient(m_initOrient[m_mode] * m_viewOrient);

	m_camera->SetFovAng(m_origFov * m_zoomPct);

	CameraController::Update();
}

void InternalCameraController::getRots(double &rX, double &rY)
{
	rX = m_rotX;
	rY = m_rotY;
}

void InternalCameraController::SetSmoothingEnabled(bool enabled)
{
	m_smoothing = enabled;
}

void InternalCameraController::SetMode(Mode m)
{
	if (m >= MODE_MAX) return;
	m_mode = m;
	SetPosition(m_initPos[m_mode]);

	static const char *m_names[6]{
		Lang::CAMERA_FRONT_VIEW,
		Lang::CAMERA_REAR_VIEW,
		Lang::CAMERA_LEFT_VIEW,
		Lang::CAMERA_RIGHT_VIEW,
		Lang::CAMERA_TOP_VIEW,
		Lang::CAMERA_BOTTOM_VIEW
	};

	m_name = m_names[m_mode];

	m_rotToX = m_rotX = 0;
	m_rotToY = m_rotY = 0;
}

void InternalCameraController::ZoomEvent(float amount)
{
	m_zoomPctTo = Clamp(m_zoomPctTo + amount * 2.0f * m_zoomPctTo, 0.4f, 1.0f);
}

void InternalCameraController::ZoomEventUpdate(float frameTime)
{
	AnimationCurves::Approach(m_zoomPct, m_zoomPctTo, frameTime, 10.f, 0.1f);
}

void InternalCameraController::SaveToJson(Json &jsonObj)
{
	Json internalCameraObj = Json::object(); // Create JSON object to contain internal camera data.

	internalCameraObj["mode"] = m_mode;
	internalCameraObj["rotX"] = m_rotToX;
	internalCameraObj["rotY"] = m_rotToY;

	jsonObj["internal"] = internalCameraObj; // Add internal camera object to supplied object.
}

void InternalCameraController::LoadFromJson(const Json &jsonObj)
{
	try {
		Json internalCameraObj = jsonObj["internal"];
		SetMode(internalCameraObj.value<Mode>("mode", MODE_FRONT));

		m_rotX = m_rotToX = internalCameraObj.value("rotX", 0.0f);
		m_rotY = m_rotToY = internalCameraObj.value("rotY", 0.0f);
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

void InternalCameraController::OnDeactivated()
{
	m_camera->SetFovAng(m_origFov);
}

ExternalCameraController::ExternalCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	MoveableCameraController(camera, ship),
	m_dist(200),
	m_distTo(m_dist),
	m_rotX(0),
	m_rotY(0),
	m_extOrient(matrix3x3d::Identity()),
	m_smoothed_ship_orient(Quaternionf())
{
}

void ExternalCameraController::ZoomEvent(float amount)
{
	m_distTo += 5 * amount * m_distTo;
	// make sure the camera can't go inside the ship or more than 1.5km away
	m_distTo = std::max(GetShip()->GetClipRadius(), std::min(GetShip()->GetClipRadius() + 1500.0, m_distTo));
}

void ExternalCameraController::ZoomEventUpdate(float frameTime)
{
	// do nothing, ZoomEventUpdate is deprecated
}

void ExternalCameraController::Reset()
{
	m_dist = 200;
	m_distTo = m_dist;
	m_smoothed_ship_orient = Quaternionf::FromMatrix3x3(GetShip()->GetInterpOrientRelTo(Pi::game->GetSpace()->GetRootFrame()));
}

void ExternalCameraController::Update()
{
	// when landed don't let external view look from below
	// XXX shouldn't be limited to player
	const Ship *ship = GetShip();
	if (ship->IsType(ObjectType::PLAYER)) {
		if (ship->GetFlightState() == Ship::LANDED ||
			ship->GetFlightState() == Ship::DOCKED) {
			m_rotX = Clamp(m_rotX, DEG2RAD(-170.0), DEG2RAD(-5.0));
		}
	}

	matrix3x3d rotMatrix = matrix3x3d::RotateY(-m_rotY) * matrix3x3d::RotateX(-m_rotX);
	vector3d dir = rotMatrix * vector3d(0, 0, 1); // ship-space camera direction

	const matrix3x3d &shipOrient = GetShip()->GetInterpOrient();

	// This lerping factor feels nice and scales non-linearly with larger (and slower-turning) ships
	float lerp_factor = (2.5 + GetShip()->GetClipRadius() * 0.05) * Pi::GetFrameTime();
	// Smooth the ship orientation by lerping toward the current (frame-invariant) orientation
	Quaternionf frame_quat = Quaternionf::FromMatrix3x3(Frame::GetFrame(GetShip()->GetFrame())->GetOrientRelTo(Pi::game->GetSpace()->GetRootFrame()));
	m_smoothed_ship_orient = Quaternionf::Slerp(m_smoothed_ship_orient, frame_quat * Quaternionf::FromMatrix3x3(shipOrient), lerp_factor).Normalized();
	matrix3x3d smoothed_m = shipOrient.Transpose() * (~frame_quat * m_smoothed_ship_orient).ToMatrix3x3<double>();
	// renormalize to remove any artifacts causing jitter
	smoothed_m.Renormalize();

	// de-penetrate the camera from any world objects it might be clipping into
	double max_dist = m_distTo;
	CollisionSpace *cspace = ship->IsInSpace() ? Frame::GetFrame(ship->GetFrame())->GetCollisionSpace() : nullptr;
	if (cspace) {
		// to check if we will end up inside an object, trace from the camera's target position towards
		// the ship - if the first thing we hit is a normal that's facing away from us; we're inside an object

		// idealPosition is in ship space
		vector3d idealPosition = smoothed_m * (dir * m_distTo);
		vector3d rayDirection = ship->GetOrient() * (-idealPosition).Normalized();

		CollisionContact contact;
		cspace->TraceRay(ship->GetOrient() * idealPosition + ship->GetPosition(), rayDirection, m_distTo, &contact, GetShip()->GetGeom());

		// userData1 will be set if we hit something
		if (contact.userData1) {
			// simple v dot n; if the result is greater than zero, we're on the wrong side of the normal
			if (contact.normal.Dot(rayDirection) > 0)
				// set the max dist to just outside the contact; for our purposes this is just fine
				max_dist = m_distTo - (contact.distance + 0.1);
		}
	}

	AnimationCurves::Approach(m_dist, std::min(m_distTo, max_dist), Pi::GetFrameTime());
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);

	SetPosition(smoothed_m * (dir * m_dist));
	SetOrient(smoothed_m * rotMatrix);

	CameraController::Update();
}

void ExternalCameraController::SaveToJson(Json &jsonObj)
{
	Json externalCameraObj = Json::object(); // Create JSON object to contain external camera data.

	externalCameraObj["rot_x"] = m_rotX;
	externalCameraObj["rot_y"] = m_rotY;
	externalCameraObj["dist"] = m_dist;

	jsonObj["external"] = externalCameraObj; // Add external camera object to supplied object.
}

void ExternalCameraController::LoadFromJson(const Json &jsonObj)
{
	try {
		Json externalCameraObj = jsonObj["external"];

		m_rotX = externalCameraObj["rot_x"];
		m_rotY = externalCameraObj["rot_y"];
		m_dist = externalCameraObj["dist"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	m_distTo = m_dist;
}

SiderealCameraController::SiderealCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	MoveableCameraController(camera, ship),
	m_dist(200),
	m_distTo(m_dist),
	m_sidOrient(matrix3x3d::Identity())
{
}

void SiderealCameraController::ZoomEvent(float amount)
{
	m_distTo += 5 * amount * m_distTo;
	m_distTo = std::max(GetShip()->GetClipRadius(), m_distTo);
}

void SiderealCameraController::ZoomEventUpdate(float frameTime)
{
	AnimationCurves::Approach(m_dist, m_distTo, frameTime, 4.0, 50. / std::max(m_distTo, 1e-7)); // std::max() here just avoid dividing by 0.
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
}

void SiderealCameraController::Reset()
{
	m_dist = 200;
	m_distTo = m_dist;
}

void SiderealCameraController::Update()
{
	const Ship *ship = GetShip();

	m_sidOrient.Renormalize(); // lots of small rotations
	matrix3x3d shipOrient = ship->GetInterpOrientRelTo(Pi::game->GetSpace()->GetRootFrame());

	SetPosition(shipOrient.Transpose() * m_sidOrient.VectorZ() * m_dist);
	SetOrient(shipOrient.Transpose() * m_sidOrient);

	CameraController::Update();
}

void SiderealCameraController::SaveToJson(Json &jsonObj)
{
	Json siderealCameraObj = Json::object(); // Create JSON object to contain sidereal camera data.

	MatrixToJson(siderealCameraObj["sid_orient"], m_sidOrient);
	siderealCameraObj["dist"] = m_dist;

	jsonObj["sidereal"] = siderealCameraObj; // Add sidereal camera object to supplied object.
}

void SiderealCameraController::LoadFromJson(const Json &jsonObj)
{
	try {
		Json siderealCameraObj = jsonObj["sidereal"];

		JsonToMatrix(&m_sidOrient, siderealCameraObj["sid_orient"]);
		m_dist = siderealCameraObj["dist"];
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	m_distTo = m_dist;
}

FlyByCameraController::FlyByCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	MoveableCameraController(camera, ship),
	m_dist(500),
	m_distTo(m_dist),
	m_roll(0),
	m_flybyOrient(matrix3x3d::Identity())
{
}

void FlyByCameraController::ZoomEvent(float amount)
{
	m_distTo += 5 * amount * m_distTo;
	m_distTo = std::max(GetShip()->GetClipRadius(), m_distTo);
}

void FlyByCameraController::ZoomEventUpdate(float frameTime)
{
	AnimationCurves::Approach(m_dist, m_distTo, frameTime, 4.0, 50. / std::max(m_distTo, 1e-7)); // std::max() here just avoid dividing by 0.
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
}

void FlyByCameraController::Reset()
{
	m_dist = 500;
	m_distTo = m_dist;
	SetPosition(vector3d(0, 0, 0));
}

void FlyByCameraController::Update()
{
	const Ship *ship = GetShip();

	matrix3x3d camerao;
	vector3d ship_pos = ship->GetInterpPosition();
	vector3d camerap;

	Frame *shipFrame = Frame::GetFrame(ship->GetFrame());
	if (GetPosition() == vector3d(0, 0, 0) || m_old_frame != shipFrame) {
		m_old_pos = ship_pos;
		m_old_frame = shipFrame;
	}

	m_flybyOrient.Renormalize();
	camerap = m_old_pos + m_flybyOrient.VectorZ() * m_dist;
	SetPosition(camerap);
	camerao = MathUtil::LookAt(camerap, ship_pos, vector3d(0, 1, 0));
	const vector3d rotAxis = camerao.VectorZ();
	camerao = matrix3x3d::Rotate(m_roll, rotAxis) * camerao;
	SetOrient(camerao);

	CameraController::Update();
}

void FlyByCameraController::SaveToJson(Json &jsonObj)
{
	Json flybyCameraObj = Json::object(); // Create JSON object to contain flyby camera data.

	flybyCameraObj["roll"] = m_roll;
	flybyCameraObj["dist"] = m_dist;
	MatrixToJson(flybyCameraObj["flyby_orient"], m_flybyOrient);

	jsonObj["flyby"] = flybyCameraObj; // Add flyby camera object to supplied object.
}

void FlyByCameraController::LoadFromJson(const Json &jsonObj)
{
	try {
		Json flybyCameraObj = jsonObj["flyby"];

		m_roll = flybyCameraObj["roll"];
		m_dist = flybyCameraObj["dist"];

		JsonToMatrix(&m_flybyOrient, flybyCameraObj["flyby_orient"]);
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}

	m_distTo = m_dist;
}
