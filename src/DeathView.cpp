// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "DeathView.h"
#include "Pi.h"
#include "Player.h"
#include "ShipCpanel.h"

DeathView::DeathView(): View()
{
	float size[2];
	GetSizeRequested(size);

	SetTransparency(true);

	float znear;
	float zfar;
	Pi::renderer->GetNearFarRange(znear, zfar);

	const float fovY = Pi::config->Float("FOVVertical");
	m_cam = new Camera(Pi::player, Pi::GetScrWidth(), Pi::GetScrHeight(), fovY, znear, zfar);
}

DeathView::~DeathView() {}

void DeathView::OnSwitchTo()
{
	m_cameraDist = Pi::player->GetBoundingRadius();
	m_cam->SetPosition(vector3d(0, 0, m_cameraDist));
	m_cam->SetOrientation(matrix4x4d::Identity());
	Pi::cpan->HideAll();
}

void DeathView::Update()
{
	assert(Pi::player->IsDead());

	m_cameraDist += 160.0 * Pi::GetFrameTime();
	m_cam->SetPosition(vector3d(0, 0, m_cameraDist));
	m_cam->Update();
}

void DeathView::Draw3D()
{
	m_cam->Draw(m_renderer);
}
