// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ModelSpinner.h"
#include "Ship.h"
#include "Pi.h"
#include "Game.h"
#include "scenegraph/Model.h"

using namespace UI;

namespace GameUI {

ModelSpinner::ModelSpinner(Context *context, SceneGraph::Model *model, const SceneGraph::ModelSkin &skin) : Widget(context),
	m_skin(skin),
	m_rotX(0), m_rotY(0),
	m_rightMouseButton(false)
{
	m_model.Reset(model->MakeInstance());
	m_skin.Apply(m_model.Get());

	Color lc(1.f);
	m_light.SetDiffuse(lc);
	m_light.SetSpecular(lc);
	m_light.SetPosition(vector3f(1.f, 1.f, 0.f));
	m_light.SetType(Graphics::Light::LIGHT_DIRECTIONAL);
}

void ModelSpinner::Layout()
{
	Point size(GetSize());
	Point activeArea(std::min(size.x, size.y));
	Point activeOffset(std::max(0, (size.x-activeArea.x)/2), std::max(0, (size.y-activeArea.y)/2));
	SetActiveArea(activeArea, activeOffset);
}

void ModelSpinner::Update()
{
	if (!(m_rightMouseButton && IsMouseActive())) {
		m_rotX += .5*Pi::GetFrameTime();
		m_rotY += Pi::GetFrameTime();
	}
}

void ModelSpinner::Draw()
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
	m_model->Render(rot);
}

void ModelSpinner::HandleMouseDown(const MouseButtonEvent &event)
{
	m_rightMouseButton = event.button == UI::MouseButtonEvent::BUTTON_RIGHT;
}

void ModelSpinner::HandleMouseMove(const UI::MouseMotionEvent &event)
{
	if (m_rightMouseButton && IsMouseActive()) {
		m_rotX += -0.002*event.rel.y;
		m_rotY += -0.002*event.rel.x;
	}
}


}
