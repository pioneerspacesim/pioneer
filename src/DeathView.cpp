// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "DeathView.h"

#include "Camera.h"
#include "Game.h"
#include "GameConfig.h"
#include "Pi.h"
#include "Player.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

DeathView::DeathView(Game *game) :
	View(),
	m_game(game)
{
	float znear;
	float zfar;
	Pi::renderer->GetNearFarRange(znear, zfar);

	const float fovY = Pi::config->Float("FOVVertical");
	m_cameraContext.Reset(new CameraContext(Pi::renderer->GetWindowWidth(), Pi::renderer->GetWindowHeight(), fovY, znear, zfar));
	m_camera.reset(new Camera(m_cameraContext, Pi::renderer));
}

DeathView::~DeathView() {}

void DeathView::Init()
{
	m_cameraDist = Pi::player->GetClipRadius() * 5.0;
	m_cameraContext->SetCameraFrame(Pi::player->GetFrame());
	m_cameraContext->SetCameraPosition(Pi::player->GetInterpPosition() + vector3d(0, 0, m_cameraDist));
	m_cameraContext->SetCameraOrient(matrix3x3d::Identity());
}

void DeathView::OnSwitchTo()
{
}

void DeathView::Update()
{
	assert(Pi::player->IsDead());

	m_cameraDist += 160.0 * Pi::GetFrameTime();
	m_cameraContext->SetCameraPosition(Pi::player->GetInterpPosition() + vector3d(0, 0, m_cameraDist));
	m_cameraContext->BeginFrame();
	m_camera->Update();
}

void DeathView::Draw3D()
{
	PROFILE_SCOPED()
	m_cameraContext->ApplyDrawTransforms(Pi::renderer);
	m_camera->Draw();
	m_cameraContext->EndFrame();
}
