// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "CameraController.h"
#include "AnimationCurves.h"
#include "Game.h"
#include "GameSaveError.h"
#include "MathUtil.h"
#include "Pi.h"
#include "Ship.h"

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
	m_camera->SetFrame(m_ship->GetFrame());
	if (GetType() == FLYBY) {
		m_camera->SetOrient(m_orient);
		m_camera->SetPosition(m_pos);
	} else {
		// interpolate between last physics tick position and current one,
		// to remove temporal aliasing
		const matrix3x3d &m = m_ship->GetInterpOrient();
		m_camera->SetOrient(m * m_orient);
		m_camera->SetPosition(m * m_pos + m_ship->GetInterpPosition());
	}
}

InternalCameraController::InternalCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	MoveableCameraController(camera, ship),
	m_mode(MODE_FRONT),
	m_rotX(0),
	m_rotY(0),
	m_intOrient(matrix3x3d::Identity()),
	m_viewOrient(matrix3x3d::Identity())
{
	Reset();
}

static bool FillCameraPosOrient(const SceneGraph::Model *m, const char *tag, vector3d &pos, matrix3x3d &orient, matrix4x4f &trans, const matrix3x3d &fallbackOrient)
{
	matrix3x3d fixOrient(matrix3x3d::Identity());

	const SceneGraph::MatrixTransform *mt = m->FindTagByName(tag);
	if (!mt) {
		fixOrient = fallbackOrient;
	} else {
		// camera points are have +Z pointing out of the ship, X left, so we
		// have to rotate 180 about Y to get them to -Z forward, X right like
		// the rest of the ship. this is not a bug, but rather a convenience to
		// modellers. it makes sense to orient the camera point in the
		// direction the camera will face
		trans = mt->GetTransform() * matrix4x4f::RotateYMatrix(M_PI);
	}

	pos = vector3d(trans.GetTranslate());

	// XXX sigh, this madness has to stop
	const matrix3x3f tagOrient = trans.GetOrient();
	matrix3x3d tagOrientd;
	matrix3x3ftod(tagOrient, tagOrientd);
	orient = fixOrient * tagOrientd;

	return true;
}

void InternalCameraController::Reset()
{
	CameraController::Reset();

	const SceneGraph::Model *m = GetShip()->GetModel();

	matrix4x4f fallbackTransform = matrix4x4f::Translation(vector3f(0.0));
	const SceneGraph::MatrixTransform *fallback = m->FindTagByName("tag_camera");
	if (fallback)
		fallbackTransform = fallback->GetTransform() * matrix4x4f::RotateYMatrix(M_PI);

	FillCameraPosOrient(m, "tag_camera_front", m_frontPos, m_frontOrient, fallbackTransform, matrix3x3d::Identity());
	FillCameraPosOrient(m, "tag_camera_rear", m_rearPos, m_rearOrient, fallbackTransform, matrix3x3d::RotateY(M_PI));
	FillCameraPosOrient(m, "tag_camera_left", m_leftPos, m_leftOrient, fallbackTransform, matrix3x3d::RotateY((M_PI / 2) * 3));
	FillCameraPosOrient(m, "tag_camera_right", m_rightPos, m_rightOrient, fallbackTransform, matrix3x3d::RotateY(M_PI / 2));
	FillCameraPosOrient(m, "tag_camera_top", m_topPos, m_topOrient, fallbackTransform, matrix3x3d::RotateX((M_PI / 2) * 3));
	FillCameraPosOrient(m, "tag_camera_bottom", m_bottomPos, m_bottomOrient, fallbackTransform, matrix3x3d::RotateX(M_PI / 2));

	SetMode(m_mode);
}

void InternalCameraController::Update()
{
	m_viewOrient =
		matrix3x3d::RotateY(-DEG2RAD(m_rotY)) *
		matrix3x3d::RotateX(-DEG2RAD(m_rotX));

	SetOrient(m_intOrient * m_viewOrient);

	CameraController::Update();
}

void InternalCameraController::getRots(double &rX, double &rY)
{
	rX = DEG2RAD(m_rotX);
	rY = DEG2RAD(m_rotY);
}

void InternalCameraController::SetMode(Mode m)
{
	m_mode = m;
	switch (m_mode) {
	case MODE_FRONT:
		m_name = Lang::CAMERA_FRONT_VIEW;
		SetPosition(m_frontPos);
		m_intOrient = m_frontOrient;
		break;
	case MODE_REAR:
		m_name = Lang::CAMERA_REAR_VIEW;
		SetPosition(m_rearPos);
		m_intOrient = m_rearOrient;
		break;
	case MODE_LEFT:
		m_name = Lang::CAMERA_LEFT_VIEW;
		SetPosition(m_leftPos);
		m_intOrient = m_leftOrient;
		break;
	case MODE_RIGHT:
		m_name = Lang::CAMERA_RIGHT_VIEW;
		SetPosition(m_rightPos);
		m_intOrient = m_rightOrient;
		break;
	case MODE_TOP:
		m_name = Lang::CAMERA_TOP_VIEW;
		SetPosition(m_topPos);
		m_intOrient = m_topOrient;
		break;
	case MODE_BOTTOM:
		m_name = Lang::CAMERA_BOTTOM_VIEW;
		SetPosition(m_bottomPos);
		m_intOrient = m_bottomOrient;
		break;
	}
	m_rotX = 0;
	m_rotY = 0;
}

void InternalCameraController::RotateUp(float frameTime)
{
	m_rotX -= 45.0f * frameTime;
}

void InternalCameraController::RotateDown(float frameTime)
{
	m_rotX += 45.0f * frameTime;
}

void InternalCameraController::RotateLeft(float frameTime)
{
	m_rotY -= 45.0f * frameTime;
}

void InternalCameraController::RotateRight(float frameTime)
{
	m_rotY += 45.0f * frameTime;
}

void InternalCameraController::SaveToJson(Json &jsonObj)
{
	Json internalCameraObj = Json::object(); // Create JSON object to contain internal camera data.

	internalCameraObj["mode"] = m_mode;

	jsonObj["internal"] = internalCameraObj; // Add internal camera object to supplied object.
}

void InternalCameraController::LoadFromJson(const Json &jsonObj)
{
	try {
		Json internalCameraObj = jsonObj["internal"];
		SetMode(internalCameraObj["mode"].get<Mode>());
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

ExternalCameraController::ExternalCameraController(RefCountedPtr<CameraContext> camera, const Ship *ship) :
	MoveableCameraController(camera, ship),
	m_dist(200),
	m_distTo(m_dist),
	m_rotX(0),
	m_rotY(0),
	m_extOrient(matrix3x3d::Identity())
{
}

void ExternalCameraController::RotateUp(float frameTime)
{
	m_rotX -= 45 * frameTime;
}

void ExternalCameraController::RotateDown(float frameTime)
{
	m_rotX += 45 * frameTime;
}

void ExternalCameraController::RotateLeft(float frameTime)
{
	m_rotY -= 45 * frameTime;
}

void ExternalCameraController::RotateRight(float frameTime)
{
	m_rotY += 45 * frameTime;
}

void ExternalCameraController::ZoomIn(float frameTime)
{
	ZoomOut(-frameTime);
}

void ExternalCameraController::ZoomOut(float frameTime)
{
	m_dist += 400 * frameTime;
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
	m_distTo = m_dist;
}

void ExternalCameraController::ZoomEvent(float amount)
{
	m_distTo += 400 * amount;
	m_distTo = std::max(GetShip()->GetClipRadius(), m_distTo);
}

void ExternalCameraController::ZoomEventUpdate(float frameTime)
{
	AnimationCurves::Approach(m_dist, m_distTo, frameTime);
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
}

void ExternalCameraController::Reset()
{
	m_dist = 200;
	m_distTo = m_dist;
}

void ExternalCameraController::Update()
{
	// when landed don't let external view look from below
	// XXX shouldn't be limited to player
	const Ship *ship = GetShip();
	if (ship->IsType(Object::PLAYER)) {
		if (ship->GetFlightState() == Ship::LANDED ||
			ship->GetFlightState() == Ship::DOCKED) {
			m_rotX = Clamp(m_rotX, -170.0, -10.0);
		}
	}

	vector3d p = vector3d(0, 0, m_dist);
	p = matrix3x3d::RotateX(-DEG2RAD(m_rotX)) * p;
	p = matrix3x3d::RotateY(-DEG2RAD(m_rotY)) * p;
	m_extOrient = matrix3x3d::RotateY(-DEG2RAD(m_rotY)) *
		matrix3x3d::RotateX(-DEG2RAD(m_rotX));

	SetPosition(p);
	SetOrient(m_extOrient);

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

void SiderealCameraController::RotateUp(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorX();
	m_sidOrient = matrix3x3d::Rotate(-M_PI / 4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::RotateDown(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorX();
	m_sidOrient = matrix3x3d::Rotate(M_PI / 4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::RotateLeft(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorY();
	m_sidOrient = matrix3x3d::Rotate(-M_PI / 4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::RotateRight(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorY();
	m_sidOrient = matrix3x3d::Rotate(M_PI / 4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::ZoomIn(float frameTime)
{
	ZoomOut(-frameTime);
}

void SiderealCameraController::ZoomOut(float frameTime)
{
	m_dist += 400 * frameTime;
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
	m_distTo = m_dist;
}

void SiderealCameraController::ZoomEvent(float amount)
{
	m_distTo += 400 * amount;
	m_distTo = std::max(GetShip()->GetClipRadius(), m_distTo);
}

void SiderealCameraController::ZoomEventUpdate(float frameTime)
{
	AnimationCurves::Approach(m_dist, m_distTo, frameTime, 4.0, 50. / std::max(m_distTo, 1e-7)); // std::max() here just avoid dividing by 0.
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
}

void SiderealCameraController::RollLeft(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorZ();
	m_sidOrient = matrix3x3d::Rotate(M_PI / 4 * frameTime, rotAxis) * m_sidOrient;
}

void SiderealCameraController::RollRight(float frameTime)
{
	const vector3d rotAxis = m_sidOrient.VectorZ();
	m_sidOrient = matrix3x3d::Rotate(-M_PI / 4 * frameTime, rotAxis) * m_sidOrient;
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

void FlyByCameraController::ZoomIn(float frameTime)
{
	ZoomOut(-frameTime);
}

void FlyByCameraController::ZoomOut(float frameTime)
{
	m_dist += 400 * frameTime;
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
	m_distTo = m_dist;
}

void FlyByCameraController::ZoomEvent(float amount)
{
	m_distTo += 400 * amount;
	m_distTo = std::max(GetShip()->GetClipRadius(), m_distTo);
}

void FlyByCameraController::ZoomEventUpdate(float frameTime)
{
	AnimationCurves::Approach(m_dist, m_distTo, frameTime, 4.0, 50. / std::max(m_distTo, 1e-7)); // std::max() here just avoid dividing by 0.
	m_dist = std::max(GetShip()->GetClipRadius(), m_dist);
}

void FlyByCameraController::RollLeft(float frameTime)
{
	m_roll += M_PI / 4 * frameTime;
}

void FlyByCameraController::RollRight(float frameTime)
{
	m_roll += -M_PI / 4 * frameTime;
}

void FlyByCameraController::RotateUp(float frameTime)
{
	const vector3d rotAxis = m_flybyOrient.VectorX();
	m_flybyOrient = matrix3x3d::Rotate(-M_PI / 4 * frameTime, rotAxis) * m_flybyOrient;
}

void FlyByCameraController::RotateDown(float frameTime)
{
	const vector3d rotAxis = m_flybyOrient.VectorX();
	m_flybyOrient = matrix3x3d::Rotate(M_PI / 4 * frameTime, rotAxis) * m_flybyOrient;
}

void FlyByCameraController::RotateLeft(float frameTime)
{
	const vector3d rotAxis = m_flybyOrient.VectorY();
	m_flybyOrient = matrix3x3d::Rotate(-M_PI / 4 * frameTime, rotAxis) * m_flybyOrient;
}

void FlyByCameraController::RotateRight(float frameTime)
{
	const vector3d rotAxis = m_flybyOrient.VectorY();
	m_flybyOrient = matrix3x3d::Rotate(M_PI / 4 * frameTime, rotAxis) * m_flybyOrient;
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

	if (GetPosition() == vector3d(0, 0, 0) || m_old_frame != ship->GetFrame()) {
		m_old_pos = ship_pos;
		m_old_frame = ship->GetFrame();
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
