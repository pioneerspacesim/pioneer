#include "ShipSpinnerWidget.h"
#include "Pi.h"
#include "Game.h"
#include "Ship.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"

ShipSpinnerWidget::ShipSpinnerWidget(const ShipFlavour &flavour, float width, float height) :
	m_width(width),
	m_height(height)
{
	m_model = LmrLookupModelByName(ShipType::types[flavour.type].lmrModelName.c_str());

	memset(&m_params, 0, sizeof(LmrObjParams));
	m_params.animationNamespace = "ShipAnimation";
	m_params.equipment = &m_equipment;
	flavour.ApplyTo(&m_params);
	m_params.animValues[Ship::ANIM_WHEEL_STATE] = 1.0;
	m_params.flightState = Ship::FLYING;

	Color lc(0.5f, 0.5f, 0.5f, 0.f);
	m_light.SetDiffuse(lc);
	m_light.SetAmbient(lc);
	m_light.SetSpecular(lc);
	m_light.SetPosition(vector3f(1.f, 1.f, 0.f));
	m_light.SetType(Graphics::Light::LIGHT_DIRECTIONAL);
}

void ShipSpinnerWidget::Draw()
{
	float pos[2];
	GetAbsolutePosition(pos);

	m_params.time = Pi::game->GetTime();

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

	m_model->Render(rot, &m_params);
}
