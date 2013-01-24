// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipSpinner.h"
#include "Ship.h"
#include "Pi.h"
#include "Game.h"

using namespace UI;

namespace GameUI {

ShipSpinner::ShipSpinner(Context *context, const ShipFlavour &flavour) : Widget(context),
	m_flavour(flavour),
	m_rotX(0), m_rotY(0)
{
	m_model = Pi::FindModel(ShipType::types[m_flavour.id].lmrModelName.c_str());

	memset(&m_params, 0, sizeof(LmrObjParams));
	m_params.animationNamespace = "ShipAnimation";
	m_params.equipment = &m_equipment;
	m_params.animValues[Ship::ANIM_WHEEL_STATE] = 1.0;
	m_params.flightState = Ship::FLYING;

	m_flavour.ApplyTo(&m_params);
	m_flavour.ApplyTo(m_model);

	Color lc(1.f);
	m_light.SetDiffuse(lc);
	m_light.SetSpecular(lc);
	m_light.SetPosition(vector3f(1.f, 1.f, 0.f));
	m_light.SetType(Graphics::Light::LIGHT_DIRECTIONAL);
}

void ShipSpinner::Layout()
{
	Point size(GetSize());
	Point activeArea(std::min(size.x, size.y));
	Point activeOffset(std::max(0, (size.x-activeArea.x)/2), std::max(0, (size.y-activeArea.y)/2));
	SetActiveArea(activeArea, activeOffset);
}

void ShipSpinner::Update()
{
	m_params.time = double(SDL_GetTicks()) * 0.001;

	if (!(m_rightMouseButton && IsMouseActive())) {
		m_rotX += .5*Pi::GetFrameTime();
		m_rotY += Pi::GetFrameTime();
	}
}

void ShipSpinner::Draw()
{

	Graphics::Renderer *r = GetContext()->GetRenderer();

	Graphics::Renderer::StateTicket ticket(r);

	r->SetPerspectiveProjection(45.f, 1.f, 1.f, 10000.f);
	r->SetTransform(matrix4x4f::Identity());

	r->SetDepthTest(true);
	r->ClearDepthBuffer();

	r->SetLights(1, &m_light);

	Point pos(GetAbsolutePosition() + GetActiveOffset());
	Point size(GetActiveArea());

	r->SetViewport(pos.x, GetContext()->GetSize().y - pos.y - size.y, size.x, size.y);

	matrix4x4f rot = matrix4x4f::RotateXMatrix(m_rotX);
	rot.RotateY(m_rotY);
	rot[14] = -1.5f * m_model->GetDrawClipRadius();
	m_model->Render(r, rot, &m_params);
}

void ShipSpinner::HandleMouseDown(const MouseButtonEvent &event)
{
	m_rightMouseButton = event.button == UI::MouseButtonEvent::BUTTON_RIGHT;
}

void ShipSpinner::HandleMouseMove(const UI::MouseMotionEvent &event)
{
	if (m_rightMouseButton && IsMouseActive()) {
		m_rotX += -0.002*event.rel.y;
		m_rotY += -0.002*event.rel.x;
	}
}


}
