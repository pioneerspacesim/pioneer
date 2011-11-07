#include "ShipSpinnerElement.h"

#include "ShipFlavour.h"
#include "Ship.h"
#include "Pi.h"
#include "render/Render.h"

bool ShipSpinnerElement::GetIntrinsicDimensions(Rocket::Core::Vector2f &dimensions)
{
	dimensions.x = 256.0f;
	dimensions.y = 256.0f;
	return true;
}

void ShipSpinnerElement::UpdateFromStash(const ShipFlavour &flavour)
{
	m_model = LmrLookupModelByName(ShipType::types[flavour.type].lmrModelName.c_str());

	memset(&m_params, 0, sizeof(LmrObjParams));
	m_params.animationNamespace = "ShipAnimation";
	m_params.equipment = &m_equipment;
	flavour.ApplyTo(&m_params);
	m_params.animValues[Ship::ANIM_WHEEL_STATE] = 1.0;
	m_params.flightState = Ship::FLYING;
}

void ShipSpinnerElement::OnRender()
{
	if (!m_model) return;

	float x1 = GetAbsoluteLeft();
	float y1 = GetAbsoluteTop();
	float x2 = x1 + GetClientWidth();
	float y2 = y1 + GetClientHeight();

	m_params.time = Pi::GetGameTime();

	m_rotX += .5*Pi::GetFrameTime();
	m_rotY += Pi::GetFrameTime();

	Render::State::SetZnearZfar(1.0f, 10000.0f);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glFrustum(-.5, .5, -.5, .5, 1.0f, 10000.0f);
	glDepthRange (0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	float lightCol[] = { .5,.5,.5,0 };
	float lightDir[] = { 1,1,0,0 };

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	glLightfv(GL_LIGHT0, GL_POSITION, lightDir);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, lightCol);
	glLightfv(GL_LIGHT0, GL_AMBIENT, lightCol);
	glLightfv(GL_LIGHT0, GL_SPECULAR, lightCol);
	glEnable(GL_LIGHT0);

	glViewport(x1, GLint(GetContext()->GetDimensions().y-y2), GLsizei(x2-x1), GLsizei(y2-y1));
	
	matrix4x4f rot = matrix4x4f::RotateXMatrix(m_rotX);
	rot.RotateY(m_rotY);
	rot[14] = -1.5f * m_model->GetDrawClipRadius();

	m_model->Render(rot, &m_params);
	Render::State::UseProgram(0);
	Render::UnbindAllBuffers();

	glPopAttrib();

	glDisable(GL_LIGHT0);
	glDisable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}
