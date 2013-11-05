// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ShipSpinnerWidget.h"
#include "Pi.h"
#include "Game.h"
#include "Ship.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "scenegraph/SceneGraph.h"

ShipSpinnerWidget::ShipSpinnerWidget(SceneGraph::Model *model, const SceneGraph::ModelSkin &skin, float width, float height) :
	m_width(width),
	m_height(height),
	m_skin(skin)
{
	m_model.reset(model->MakeInstance());
	m_skin.Apply(m_model.get());

	Color lc(1.f);
	m_light.SetDiffuse(lc);
	m_light.SetSpecular(lc);
	m_light.SetPosition(vector3f(1.f, 1.f, 0.f));
	m_light.SetType(Graphics::Light::LIGHT_DIRECTIONAL);
}

void ShipSpinnerWidget::Draw()
{
	float pos[2];
	GetAbsolutePosition(pos);

	float guiscale[2];
	Gui::Screen::GetCoords2Pixels(guiscale);
	static float rot1, rot2;
	if (Pi::MouseButtonState(SDL_BUTTON_RIGHT)) {
		int m[2];
		Pi::GetMouseMotion(m);
		rot1 += -0.002*m[1];
		rot2 += -0.002*m[0];
	}
	else
	{
		rot1 += .5*Pi::GetFrameTime();
		rot2 += Pi::GetFrameTime();
	}

	glColor3f(0,0,0);
	glBegin(GL_QUADS);
		glVertex2f(0.0f, 0.0f);
		glVertex2f(0.0f, m_height);
		glVertex2f(m_width, m_height);
		glVertex2f(m_width, 0.0f);
	glEnd();

	Graphics::Renderer::StateTicket ticket(Pi::renderer);

	Pi::renderer->SetPerspectiveProjection(45.f, 1.f, 1.f, 10000.f);
	Pi::renderer->SetTransform(matrix4x4f::Identity());

	Pi::renderer->SetDepthWrite(true);
	Pi::renderer->SetDepthTest(true);
	Pi::renderer->ClearDepthBuffer();

	Pi::renderer->SetLights(1, &m_light);
	Pi::renderer->SetViewport(
		int(roundf(pos[0]/guiscale[0])),
		int(roundf((Gui::Screen::GetHeight() - pos[1] - m_height)/guiscale[1])),
		int(m_width/guiscale[0]),
		int(m_height/guiscale[1]));

	matrix4x4f rot = matrix4x4f::RotateXMatrix(rot1);
	rot.RotateY(rot2);
	rot[14] = -1.5f * m_model->GetDrawClipRadius();

	m_model->Render(rot);
}
