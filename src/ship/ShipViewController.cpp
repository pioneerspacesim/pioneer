// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipViewController.h"

#include "CameraController.h"
#include "GameSaveError.h"
#include "Input.h"
#include "WorldView.h"

#include "Pi.h"
#include "Player.h"
#include "PlayerShipController.h"

namespace {
	static const float MOUSELOOK_SPEED = 0.01;
	static const float ZOOM_SPEED = 1.f;
	static const float WHEEL_SENSITIVITY = .05f; // Should be a variable in user settings.
} // namespace

REGISTER_INPUT_BINDING(ShipViewController)
{
	using namespace InputBindings;

	Input::BindingGroup *group = input->GetBindingPage("ShipView")->GetBindingGroup("GeneralViewControls");

	input->AddAxisBinding("BindCameraRoll", group, Axis({}, { SDLK_KP_1 }, { SDLK_KP_3 }));
	input->AddAxisBinding("BindCameraPitch", group, Axis({}, { SDLK_KP_2 }, { SDLK_KP_8 }));
	input->AddAxisBinding("BindCameraYaw", group, Axis({}, { SDLK_KP_4 }, { SDLK_KP_6 }));
	input->AddAxisBinding("BindViewZoom", group, Axis({}, { SDLK_EQUALS }, { SDLK_MINUS }));

	input->AddAxisBinding("BindLookYaw", group, Axis());
	input->AddAxisBinding("BindLookPitch", group, Axis());

	input->AddActionBinding("BindFrontCamera", group, Action({ SDLK_KP_8 }, { SDLK_UP }));
	input->AddActionBinding("BindRearCamera", group, Action({ SDLK_KP_2 }, { SDLK_DOWN }));
	input->AddActionBinding("BindLeftCamera", group, Action({ SDLK_KP_4 }, { SDLK_LEFT }));
	input->AddActionBinding("BindRightCamera", group, Action({ SDLK_KP_6 }, { SDLK_RIGHT }));
	input->AddActionBinding("BindTopCamera", group, Action({ SDLK_KP_9 }));
	input->AddActionBinding("BindBottomCamera", group, Action({ SDLK_KP_3 }));

	input->AddActionBinding("BindCycleCameraMode", group, Action({ SDLK_F1, SDLK_LCTRL }));
	input->AddActionBinding("BindResetCamera", group, Action({ SDLK_HOME }));
}

void ShipViewController::InputBinding::RegisterBindings()
{
	cameraRoll = AddAxis("BindCameraRoll");
	cameraPitch = AddAxis("BindCameraPitch");
	cameraYaw = AddAxis("BindCameraYaw");
	cameraZoom = AddAxis("BindViewZoom");

	lookYaw = AddAxis("BindLookYaw");
	lookPitch = AddAxis("BindLookPitch");

	frontCamera = AddAction("BindFrontCamera");
	rearCamera = AddAction("BindRearCamera");
	leftCamera = AddAction("BindLeftCamera");
	rightCamera = AddAction("BindRightCamera");
	topCamera = AddAction("BindTopCamera");
	bottomCamera = AddAction("BindBottomCamera");

	cycleCameraMode = AddAction("BindCycleCameraMode");
	resetCamera = AddAction("BindResetCamera");
}

ShipViewController::ShipViewController(WorldView *v) :
	ViewController(v),
	m_camType(CAM_INTERNAL),
	headtracker_input_priority(false),
	InputBindings(Pi::input)
{
	InputBindings.RegisterBindings();
}

void ShipViewController::LoadFromJson(const Json &jsonObj)
{
	if (!jsonObj["cam_type"].is_number_integer())
		throw SavedGameCorruptException();

	SetCamType(jsonObj["cam_type"]);

	m_internalCameraController->LoadFromJson(jsonObj);
	m_externalCameraController->LoadFromJson(jsonObj);
	m_siderealCameraController->LoadFromJson(jsonObj);
	m_flybyCameraController->LoadFromJson(jsonObj);
}

void ShipViewController::SaveToJson(Json &jsonObj)
{
	jsonObj["cam_type"] = int(m_camType);
	m_internalCameraController->SaveToJson(jsonObj);
	m_externalCameraController->SaveToJson(jsonObj);
	m_siderealCameraController->SaveToJson(jsonObj);
	m_flybyCameraController->SaveToJson(jsonObj);
}

void ShipViewController::Init()
{
	RefCountedPtr<CameraContext> m_cameraContext = m_parentView->GetCameraContext();
	m_internalCameraController.reset(new InternalCameraController(m_cameraContext, Pi::player));
	m_externalCameraController.reset(new ExternalCameraController(m_cameraContext, Pi::player));
	m_siderealCameraController.reset(new SiderealCameraController(m_cameraContext, Pi::player));
	m_flybyCameraController.reset(new FlyByCameraController(m_cameraContext, Pi::player));
	SetCamType(m_camType); //set the active camera
}

void ShipViewController::Activated()
{
	Pi::input->AddInputFrame(&InputBindings);

	m_onMouseWheelCon =
		Pi::input->onMouseWheel.connect(sigc::mem_fun(this, &ShipViewController::MouseWheel));

	Pi::player->GetPlayerController()->SetMouseForRearView(GetCamType() == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR);
	Pi::player->SetFlag(Body::FLAG_DRAW_EXCLUDE, !IsExteriorView());
}

void ShipViewController::Deactivated()
{
	Pi::player->SetFlag(Body::FLAG_DRAW_EXCLUDE, false);
	Pi::input->RemoveInputFrame(&InputBindings);

	m_onMouseWheelCon.disconnect();
}

void ShipViewController::SetCamType(enum CamType c)
{
	// TODO: add collision testing for external cameras to avoid clipping through
	// stations / spaceports the ship is docked to.

	m_camType = c;

	switch (m_camType) {
	case CAM_INTERNAL:
		m_activeCameraController = m_internalCameraController.get();
		Pi::player->OnCockpitActivated();
		break;
	case CAM_EXTERNAL:
		m_activeCameraController = m_externalCameraController.get();
		break;
	case CAM_SIDEREAL:
		m_activeCameraController = m_siderealCameraController.get();
		break;
	case CAM_FLYBY:
		m_activeCameraController = m_flybyCameraController.get();
		break;
	}

	if (m_camType != CAM_INTERNAL) {
		headtracker_input_priority = false;
	}

	Pi::player->SetFlag(Body::FLAG_DRAW_EXCLUDE, !IsExteriorView());
	Pi::player->GetPlayerController()->SetMouseForRearView(m_camType == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR);

	m_activeCameraController->Reset();

	onChangeCamType.emit();
}

bool ShipViewController::IsExteriorView() const
{
	return m_camType != CAM_INTERNAL;
}

void ShipViewController::ChangeInternalCameraMode(InternalCameraController::Mode m)
{
	if (m_internalCameraController->GetMode() != m)
		// TODO: find a way around this, or move it to a dedicated system.
		Sound::PlaySfx("Click", 0.3, 0.3, false);
	m_internalCameraController->SetMode(m);
	Pi::player->GetPlayerController()->SetMouseForRearView(m_camType == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR);
}

void ShipViewController::Update()
{
	auto *cam = static_cast<MoveableCameraController *>(m_activeCameraController);
	auto frameTime = Pi::GetFrameTime();

	if (!InputBindings.active) return;

	if (GetCamType() == CAM_INTERNAL) {
		if (InputBindings.frontCamera->IsActive())
			ChangeInternalCameraMode(InternalCameraController::MODE_FRONT);
		else if (InputBindings.rearCamera->IsActive())
			ChangeInternalCameraMode(InternalCameraController::MODE_REAR);
		else if (InputBindings.leftCamera->IsActive())
			ChangeInternalCameraMode(InternalCameraController::MODE_LEFT);
		else if (InputBindings.rightCamera->IsActive())
			ChangeInternalCameraMode(InternalCameraController::MODE_RIGHT);
		else if (InputBindings.topCamera->IsActive())
			ChangeInternalCameraMode(InternalCameraController::MODE_TOP);
		else if (InputBindings.bottomCamera->IsActive())
			ChangeInternalCameraMode(InternalCameraController::MODE_BOTTOM);

		vector3f rotate = vector3f(
			InputBindings.lookPitch->GetValue() * M_PI / 2.0,
			InputBindings.lookYaw->GetValue() * M_PI / 2.0,
			0.0);

		if (rotate.LengthSqr() > 0.0001) {
			cam->SetRotationAngles(rotate);
			headtracker_input_priority = true;
		} else if (headtracker_input_priority) {
			cam->SetRotationAngles({ 0.0, 0.0, 0.0 });
			headtracker_input_priority = false;
		}
	} else {
		vector3d rotate = vector3d(
			-InputBindings.cameraPitch->GetValue(),
			InputBindings.cameraYaw->GetValue(),
			InputBindings.cameraRoll->GetValue());

		rotate *= frameTime;

		// Horribly abuse our knowledge of the internals of cam->RotateUp/Down.
		// Applied in YXZ order because reasons.
		if (rotate.y != 0.0) cam->YawCamera(rotate.y);
		if (rotate.x != 0.0) cam->PitchCamera(rotate.x);
		if (rotate.z != 0.0) cam->RollCamera(rotate.z);

		if (InputBindings.cameraZoom->IsActive())
			cam->ZoomEvent(-InputBindings.cameraZoom->GetValue() * ZOOM_SPEED * frameTime);
		if (InputBindings.resetCamera->IsActive())
			cam->Reset();
		cam->ZoomEventUpdate(frameTime);
	}

	int mouseMotion[2];
	Pi::input->GetMouseMotion(mouseMotion);

	// external camera mouselook
	bool mouse_down = Pi::input->MouseButtonState(SDL_BUTTON_MIDDLE);
	if (mouse_down && !headtracker_input_priority) {
		if (!m_mouseActive) {
			m_mouseActive = true;
			Pi::input->SetCapturingMouse(true);
		}

		// invert the mouse input to convert between screen coordinates and
		// right-hand coordinate system rotation.
		cam->YawCamera(-mouseMotion[0] * MOUSELOOK_SPEED);
		cam->PitchCamera(-mouseMotion[1] * MOUSELOOK_SPEED);
	}

	if (!mouse_down && m_mouseActive) {
		m_mouseActive = false;
		Pi::input->SetCapturingMouse(false);
	}

	m_activeCameraController->Update();
}

void ShipViewController::Draw(Camera *camera)
{
	// Render cockpit
	// XXX camera should rotate inside cockpit, not rotate the cockpit around in the world
	if (!IsExteriorView() && Pi::player->GetCockpit() && m_internalCameraController->GetMode() == InternalCameraController::MODE_FRONT)
		Pi::player->GetCockpit()->RenderCockpit(Pi::renderer, camera, camera->GetContext()->GetTempFrame());
}

void ShipViewController::MouseWheel(bool up)
{
	if (m_activeCameraController->IsExternal()) {
		MoveableCameraController *cam = static_cast<MoveableCameraController *>(m_activeCameraController);

		if (!up) // Zoom out
			cam->ZoomEvent(ZOOM_SPEED * WHEEL_SENSITIVITY);
		else
			cam->ZoomEvent(-ZOOM_SPEED * WHEEL_SENSITIVITY);
	}
}
