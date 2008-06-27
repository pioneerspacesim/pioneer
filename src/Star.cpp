#include "libs.h"
#include "Star.h"
#include "Pi.h"

Star::Star(StarSystem::SBody::SubType subtype): Body()
{
	m_subtype = subtype;
	m_radius = 6378135.0;
	m_pos = vector3d(0,0,0);
}

vector3d Star::GetPosition()
{
	return m_pos;
}

void Star::SetPosition(vector3d p)
{
	m_pos = p;
}

void Star::TransformToModelCoords(const Frame *camFrame)
{
	vector3d fpos = GetPositionRelTo(camFrame);
	glTranslatef(m_pos[0]+fpos.x, m_pos[1]+fpos.y, m_pos[2]+fpos.z);
}

void Star::Render(const Frame *a_camFrame)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
	
	/* XXX duplicates code from Planet.cpp. bad. */
	double rad = m_radius;
	vector3d fpos = GetPositionRelTo(a_camFrame);
	double len = fpos.Length();

	while (len > 1000.0f) {
		rad *= 0.25;
		fpos = 0.25*fpos;
		len *= 0.25;
	}

	glTranslatef(fpos.x, fpos.y, fpos.z);
	
	//TransformToModelCoords(a_camFrame);
	glColor3fv(StarSystem::starColors[m_subtype]);
	gluSphere(Pi::gluQuadric, rad, 100, 100);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
