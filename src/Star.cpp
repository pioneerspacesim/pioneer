#include "libs.h"
#include "Star.h"
#include "Pi.h"
#include "WorldView.h"
#include "Serializer.h"
#include "Render.h"

Star::Star(SBody *sbody): Body()
{
	m_sbody = sbody;
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

void Star::Save(Serializer::Writer &wr)
{
	Body::Save(wr);
	wr.Int32(Serializer::LookupSystemBody(m_sbody));
	wr.Vector3d(pos);
	wr.Double(radius);
	wr.Double(mass);
}

void Star::Load(Serializer::Reader &rd)
{
	Body::Load(rd);
	m_sbody = Serializer::LookupSystemBody(rd.Int32());
	pos = rd.Vector3d();
	radius = rd.Double();
	mass = rd.Double();
}

void Star::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();

	Render::State::UseProgram(0);
	
	double rad = radius;
	vector3d fpos = viewCoords;
	double len = fpos.Length();

	while (len > 1000.0f) {
		rad *= 0.25;
		fpos = 0.25*fpos;
		len *= 0.25;
	}

	glTranslatef((float)fpos.x, (float)fpos.y, (float)fpos.z);
	
	{	
		const float *col = StarSystem::starRealColors[m_sbody->type];
		// face the camera dammit
		vector3d zaxis = fpos.Normalized();
		vector3d xaxis = vector3d::Cross(vector3d(0,1,0), zaxis).Normalized();
		vector3d yaxis = vector3d::Cross(zaxis,xaxis);
		matrix4x4d rot = matrix4x4d::MakeInvRotMatrix(xaxis, yaxis, zaxis);
		glMultMatrixd(&rot[0]);

		const float glowrad = (float)(rad+0.015*radius*len/SOL_RADIUS);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);	
		glEnable(GL_BLEND);
		glBegin(GL_TRIANGLE_FAN);
		glColor4f(col[0], col[1], col[2], 1);
		glVertex3f(0, 0, 0);
		glColor4f(0,0,0,0);
		for (float ang=0; ang<2*M_PI; ang+=0.2) {
			glVertex3f(4*glowrad*sin(ang), 4*glowrad*cos(ang), 0);
		}
		glVertex3f(0, 4*glowrad, 0);
		glEnd();
		glDisable(GL_BLEND);
		
		Render::State::UseProgram(Render::simpleShader);
		glEnable(GL_BLEND);
		const float b = (Render::IsHDREnabled() ? 100.0f : 1.0f);
		glColor4f(b*col[0],b*col[1],b*col[2],1);
		glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0,0,0);
		glColor4f(0,0,0,0);
		
		const float spikerad = (float)(0.02*len + 0.1*radius*len/SOL_RADIUS);
		{
			/* cubic bezier with 2 (0,0,0) control points */
			vector3f p0(0,spikerad,0), p1(spikerad,0,0);
			float t=0.1; for (int i=1; i<10; i++, t+= 0.1f) {
				vector3f p = (1-t)*(1-t)*(1-t)*(1-t)*p0 + t*t*t*t*p1;
				glVertex3fv(&p[0]);
			}
		}
		{
			vector3f p0(spikerad,0,0), p1(0,-spikerad,0);
			float t=0.1; for (int i=1; i<10; i++, t+= 0.1f) {
				vector3f p = (1-t)*(1-t)*(1-t)*(1-t)*p0 + t*t*t*t*p1;
				glVertex3fv(&p[0]);
			}
		}
		{
			vector3f p0(0,-spikerad,0), p1(-spikerad,0,0);
			float t=0.1; for (int i=1; i<10; i++, t+= 0.1f) {
				vector3f p = (1-t)*(1-t)*(1-t)*(1-t)*p0 + t*t*t*t*p1;
				glVertex3fv(&p[0]);
			}
		}
		{
			vector3f p0(-spikerad,0,0), p1(0,spikerad,0);
			float t=0.1; for (int i=1; i<10; i++, t+= 0.1f) {
				vector3f p = (1-t)*(1-t)*(1-t)*(1-t)*p0 + t*t*t*t*p1;
				glVertex3fv(&p[0]);
			}
		}
		glEnd();
		glDisable(GL_BLEND);

		glBegin(GL_TRIANGLE_FAN);
		glColor4f(b*col[0],b*col[1],b*col[2],1);
		glVertex3f(0, 0, 0);
		for (float ang=0; ang<2*M_PI; ang+=0.1) {
			glVertex3f((float)(rad*sin(ang)), (float)(rad*cos(ang)), 0);
		}
		glVertex3f(0, (float)rad, 0);
		glEnd();
		Render::State::UseProgram(0);
	}
	
	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
