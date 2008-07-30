#include "libs.h"
#include "Star.h"
#include "Pi.h"
#include "WorldView.h"

Star::Star(StarSystem::SBody *sbody): Body()
{
	this->type = sbody->type;
	radius = sbody->GetRadius();
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

static void DrawCorona(double rad, vector3d &pos, const float col[3])
{
	glPushMatrix();
	// face the camera dammit
	vector3d zaxis = vector3d::Normalize(pos);
	vector3d xaxis = vector3d::Normalize(vector3d::Cross(zaxis, vector3d(0,1,0)));
	vector3d yaxis = vector3d::Cross(zaxis,xaxis);
	matrix4x4d rot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();
	glMultMatrixd(&rot[0]);

	glEnable(GL_BLEND);
	glDisable(GL_CULL_FACE);
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(col[0], col[1], col[2], 1);
	glVertex3f(0, 0, 0);
	glColor4f(0,0,0,0);
	for (float ang=0; ang<2*M_PI; ang+=0.2) {
		glVertex3f(rad*sin(ang), rad*cos(ang), 0);
	}
	glVertex3f(0, rad, 0);
	glEnd();
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glPopMatrix();
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
	DrawCorona(rad*4, fpos, StarSystem::starColors[type]);
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
