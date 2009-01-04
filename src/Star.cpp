#include "libs.h"
#include "Star.h"
#include "Pi.h"
#include "WorldView.h"
#include "Serializer.h"

Star::Star(StarSystem::SBody *sbody): Body()
{
	this->type = sbody->type;
	radius = sbody->GetRadius();
	mass = sbody->GetMass();
	pos = vector3d(0,0,0);
}

vector3d Star::GetPosition() const
{
	return pos;
}

void Star::SetPosition(vector3d p)
{
	pos = p;
}

void Star::Save()
{
	using namespace Serializer::Write;
	Body::Save();
	wr_int(type);
	wr_vector3d(pos);
	wr_double(radius);
	wr_double(mass);
}

void Star::Load()
{
	using namespace Serializer::Read;
	Body::Load();
	type = (StarSystem::BodyType)rd_int();
	pos = rd_vector3d();
	radius = rd_double();
	mass = rd_double();
}

void Star::Render(const Frame *a_camFrame)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();
	
	double rad = radius;
	vector3d fpos = GetPositionRelTo(a_camFrame);
	double len = fpos.Length();

	while (len > 1000.0f) {
		rad *= 0.25;
		fpos = 0.25*fpos;
		len *= 0.25;
	}

	glTranslatef(fpos.x, fpos.y, fpos.z);
	
	{	
		const float *col = StarSystem::starRealColors[type];
		// face the camera dammit
		vector3d zaxis = fpos.Normalized();
		vector3d xaxis = vector3d::Cross(vector3d(0,1,0), zaxis).Normalized();
		vector3d yaxis = vector3d::Cross(zaxis,xaxis);
		matrix4x4d rot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();
		glMultMatrixd(&rot[0]);

		glEnable(GL_BLEND);
		glBegin(GL_TRIANGLE_FAN);
		glColor4f(col[0], col[1], col[2], 1);
		glVertex3f(0, 0, 0);
		glColor4f(0,0,0,0);
		for (float ang=0; ang<2*M_PI; ang+=0.2) {
			glVertex3f(4*rad*sin(ang), 4*rad*cos(ang), 0);
		}
		glVertex3f(0, 4*rad, 0);
		glEnd();
		glDisable(GL_BLEND);
		
		glBegin(GL_TRIANGLE_FAN);
		glColor4f(col[0], col[1], col[2], 1);
		glVertex3f(0, 0, 0);
		for (float ang=0; ang<2*M_PI; ang+=0.1) {
			glVertex3f(rad*sin(ang), rad*cos(ang), 0);
		}
		glVertex3f(0, rad, 0);
		glEnd();
	}
	
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
