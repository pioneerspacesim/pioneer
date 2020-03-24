// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#pragma once

#include "CameraController.h"
#include "Input.h"
#include "InteractionController.h"
#include "KeyBindings.h"
#include "utils.h"

class ShipViewController : public InteractionController {
public:
	ShipViewController(WorldView *v) :
		InteractionController(v),
		m_camType(CAM_INTERNAL),
		headtracker_input_priority(false) {}

	void Update() override;
	void Activated() override;
	void Deactivated() override;

	enum CamType {
		CAM_INTERNAL,
		CAM_EXTERNAL,
		CAM_SIDEREAL,
		CAM_FLYBY
	};
	void SetCamType(enum CamType);
	enum CamType GetCamType() const { return m_camType; }
	CameraController *GetCameraController() const { return m_activeCameraController; }

	sigc::signal<void> onChangeCamType;

private:
	friend class WorldView;
	void ChangeInternalCameraMode(InternalCameraController::Mode m);

	enum CamType m_camType;

	sigc::connection m_onMouseWheelCon;

	std::unique_ptr<InternalCameraController> m_internalCameraController;
	std::unique_ptr<ExternalCameraController> m_externalCameraController;
	std::unique_ptr<SiderealCameraController> m_siderealCameraController;
	std::unique_ptr<FlyByCameraController> m_flybyCameraController;
	CameraController *m_activeCameraController; //one of the above

	bool headtracker_input_priority;
	bool m_mouseActive;

	void MouseWheel(bool up);

public:
	void Init();
	void LoadFromJson(const Json &jsonObj);
	void SaveToJson(Json &jsonObj);

	static struct InputBinding : public Input::InputFrame {
		using Action = KeyBindings::ActionBinding;
		using Axis = KeyBindings::AxisBinding;

		Axis *cameraYaw;
		Axis *cameraPitch;
		Axis *cameraRoll;
		Axis *cameraZoom;

		Axis *lookYaw;
		Axis *lookPitch;

		Action *frontCamera;
		Action *rearCamera;
		Action *leftCamera;
		Action *rightCamera;
		Action *topCamera;
		Action *bottomCamera;

		Action *cycleCameraMode;
		Action *resetCamera;

		virtual void RegisterBindings();
	} InputBindings;
};
