
#include "ShipViewController.h"
#include "CameraController.h"
#include "WorldView.h"

#include "Pi.h"
#include "Player.h"

namespace {
	static const float ZOOM_SPEED = 1.f;
	static const float WHEEL_SENSITIVITY = .05f; // Should be a variable in user settings.
} // namespace

ShipViewController::InputBinding ShipViewController::InputBindings;

void ShipViewController::InputBinding::RegisterBindings()
{
	using namespace KeyBindings;

	Input::BindingPage *page = Pi::input.GetBindingPage("VIEW");
	Input::BindingGroup *group;

#define BINDING_GROUP(n) group = page->GetBindingGroup(#n);
#define KEY_BINDING(n, id, k1, k2)                                    \
	n =                                                               \
		Pi::input.AddActionBinding(id, group, ActionBinding(k1, k2)); \
	actions.push_back(n);
#define AXIS_BINDING(n, id, k1, k2)                               \
	n =                                                           \
		Pi::input.AddAxisBinding(id, group, AxisBinding(k1, k2)); \
	axes.push_back(n);

	BINDING_GROUP(GENERAL_VIEW_CONTROLS)
	KEY_BINDING(cycleCameraMode, "BindCycleCameraMode", SDLK_F1, 0)

	AXIS_BINDING(cameraRoll, "BindCameraRoll", SDLK_KP_1, SDLK_KP_3)
	AXIS_BINDING(cameraPitch, "BindCameraPitch", SDLK_KP_2, SDLK_KP_8)
	AXIS_BINDING(cameraYaw, "BindCameraYaw", SDLK_KP_4, SDLK_KP_6)
	AXIS_BINDING(cameraZoom, "BindViewZoom", SDLK_EQUALS, SDLK_MINUS)

	KEY_BINDING(frontCamera, "BindFrontCamera", SDLK_KP_8, SDLK_UP)
	KEY_BINDING(rearCamera, "BindRearCamera", SDLK_KP_2, SDLK_DOWN)
	KEY_BINDING(leftCamera, "BindLeftCamera", SDLK_KP_4, SDLK_LEFT)
	KEY_BINDING(rightCamera, "BindRightCamera", SDLK_KP_6, SDLK_RIGHT)
	KEY_BINDING(topCamera, "BindTopCamera", SDLK_KP_9, 0)
	KEY_BINDING(bottomCamera, "BindBottomCamera", SDLK_KP_3, 0)

	KEY_BINDING(resetCamera, "BindResetCamera", SDLK_HOME, 0)

#undef BINDING_GROUP
#undef KEY_BINDING
#undef AXIS_BINDING
}

void ShipViewController::LoadFromJson(const Json &jsonObj)
{
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
	RefCountedPtr<CameraContext> m_cameraContext = parentView->GetCameraContext();
	m_internalCameraController.reset(new InternalCameraController(m_cameraContext, Pi::player));
	m_externalCameraController.reset(new ExternalCameraController(m_cameraContext, Pi::player));
	m_siderealCameraController.reset(new SiderealCameraController(m_cameraContext, Pi::player));
	m_flybyCameraController.reset(new FlyByCameraController(m_cameraContext, Pi::player));
	SetCamType(m_camType); //set the active camera
}

void ShipViewController::Activated()
{
	Pi::input.PushInputFrame(&InputBindings);

	m_onMouseWheelCon =
		Pi::input.onMouseWheel.connect(sigc::mem_fun(this, &ShipViewController::MouseWheel));

	Pi::player->GetPlayerController()->SetMouseForRearView(GetCamType() == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR);
}

void ShipViewController::Deactivated()
{
	Pi::input.RemoveInputFrame(&InputBindings);

	m_onMouseWheelCon.disconnect();
}

void ShipViewController::SetCamType(enum CamType c)
{
	Pi::BoinkNoise();

	// don't allow external cameras when docked inside space stations.
	// they would clip through the station model
	//if (Pi::player->GetFlightState() == Ship::DOCKED && !Pi::player->GetDockedWith()->IsGroundStation())
	//	c = CAM_INTERNAL;

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

	Pi::player->GetPlayerController()->SetMouseForRearView(m_camType == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR);

	m_activeCameraController->Reset();

	onChangeCamType.emit();
}

void ShipViewController::ChangeInternalCameraMode(InternalCameraController::Mode m)
{
	if (m_internalCameraController->GetMode() != m)
		Pi::BoinkNoise();
	m_internalCameraController->SetMode(m);
	Pi::player->GetPlayerController()->SetMouseForRearView(m_camType == CAM_INTERNAL && m_internalCameraController->GetMode() == InternalCameraController::MODE_REAR);
}

void ShipViewController::Update()
{
	auto *cam = static_cast<MoveableCameraController *>(m_activeCameraController);
	auto frameTime = Pi::GetFrameTime();

	vector3d rotate = vector3d(
		-InputBindings.cameraPitch->GetValue(),
		InputBindings.cameraYaw->GetValue(),
		InputBindings.cameraRoll->GetValue());

	// Horribly abuse our knowledge of the internals of cam->RotateUp/Down.
	if (rotate.x != 0.0) cam->RotateUp(frameTime * rotate.x);
	if (rotate.y != 0.0) cam->RotateLeft(frameTime * rotate.y);
	if (rotate.z != 0.0) cam->RollLeft(frameTime * rotate.z);

	// XXX ugly hack checking for console here
	if (!Pi::IsConsoleActive()) {
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
		} else {
			MoveableCameraController *cam = static_cast<MoveableCameraController *>(m_activeCameraController);
			vector3d rotate = vector3d(
				-InputBindings.cameraPitch->GetValue(),
				InputBindings.cameraYaw->GetValue(),
				InputBindings.cameraRoll->GetValue());

			// Horribly abuse our knowledge of the internals of cam->RotateUp/Down.
			if (rotate.x != 0.0) cam->RotateUp(frameTime * rotate.x);
			if (rotate.y != 0.0) cam->RotateLeft(frameTime * rotate.y);
			if (rotate.z != 0.0) cam->RollLeft(frameTime * rotate.z);

			if (InputBindings.cameraZoom->IsActive())
				cam->ZoomEvent(-InputBindings.cameraZoom->GetValue() * ZOOM_SPEED * frameTime);
			if (InputBindings.resetCamera->IsActive())
				cam->Reset();
			cam->ZoomEventUpdate(frameTime);
		}
	}

	int mouseMotion[2];
	Pi::input.GetMouseMotion(mouseMotion);

	// external camera mouselook
	if (Pi::input.MouseButtonState(SDL_BUTTON_MIDDLE)) {
		const double accel = 0.01; // XXX configurable?
		cam->RotateLeft(mouseMotion[0] * accel);
		cam->RotateUp(mouseMotion[1] * accel);
	}

	m_activeCameraController->Update();
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
