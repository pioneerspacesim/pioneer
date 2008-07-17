#include "libs.h"
#include "Star.h"
#include "Pi.h"

Star::Star(StarSystem::BodyType type): Body()
{
	this->type = type;
	radius = 6378135.0;
	pos = vector3d(0,0,0);
}

vector3d Star::GetPosition()
{
	return pos;
}

void Star::SetPosition(vector3d p)
{
	pos = p;
}

void Star::Render(const Frame *a_camFrame)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
	
	/* XXX duplicates code from Planet.cpp. bad. */
	double rad = radius;
	vector3d fpos = GetPositionRelTo(a_camFrame);
	double len = fpos.Length();

	while (len > 1000.0f) {
		rad *= 0.25;
		fpos = 0.25*fpos;
		len *= 0.25;
	}

	glTranslatef(fpos.x, fpos.y, fpos.z);
	
	glColor3fv(StarSystem::starColors[type]);
	gluSphere(Pi::gluQuadric, rad, 100, 100);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
