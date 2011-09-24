#include "Star.h"
#include "render/Render.h"
#include "gui/Gui.h"

Star::Star() : TerrainBody()
{
}

Star::Star(SBody *sbody): TerrainBody(sbody)
{
	m_hasDoubleFrame = false;
}

void Star::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();

	Render::State::UseProgram(0);

	double radius = GetSBody()->GetRadius();
	
	double rad = radius;
	vector3d fpos = viewCoords;
	double len = fpos.Length();

	while (len > 1000.0f) {
		rad *= 0.25;
		fpos = 0.25*fpos;
		len *= 0.25;
	}

	glTranslatef(float(fpos.x), float(fpos.y), float(fpos.z));
	
	if (IsOnscreen()) {
		const float *col = StarSystem::starRealColors[GetSBody()->type];
		const float b = (Render::IsHDREnabled() ? 100.0f : 1.0f);


		/* Draw star spikes and halo to 2d ortho screen */
		
		Gui::Screen::EnterOrtho();
		vector3d pp;
		Gui::Screen::Project(fpos, pp);

		const float glowrad = float(20.0f+20000.0f*radius/viewCoords.Length());
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);	
		glEnable(GL_BLEND);
		glBegin(GL_TRIANGLE_FAN);
		glColor4f(0.5f*col[0], 0.5f*col[1], 0.5f*col[2], 1);
		glVertex3f(pp.x, pp.y, 0);
		glColor4f(0,0,0,0);
		for (float ang=0; ang<2*M_PI; ang+=0.2) {
			glVertex3f(pp.x+glowrad*sin(ang), pp.y+glowrad*cos(ang), 0);
		}
		glVertex3f(pp.x, pp.y+glowrad, 0);
		glEnd();
		glDisable(GL_BLEND);
		
		Render::State::UseProgram(Render::simpleShader);
		glEnable(GL_BLEND);
		glColor4f(0.5f*b*col[0],0.5f*b*col[1],0.5f*b*col[2],1);
		glBegin(GL_TRIANGLE_FAN);
		glVertex3f(pp.x,pp.y,0);
		glColor4f(0,0,0,0);

#if 0
		const float spikerad = std::min<float>(10.0f+20000.0f*radius/viewCoords.Length(), 0.5f*float(Gui::Screen::GetHeight()));
		{
			/* cubic bezier with 2 (0,0,0) control points */
			vector3f p0(0,spikerad,0), p1(spikerad,0,0);
			float t=0.1; for (int i=1; i<10; i++, t+= 0.1f) {
				vector3f p = pp + (1-t)*(1-t)*(1-t)*(1-t)*p0 + t*t*t*t*p1;
				glVertex3fv(&p[0]);
			}
		}
		{
			vector3f p0(spikerad,0,0), p1(0,-spikerad,0);
			float t=0.1; for (int i=1; i<10; i++, t+= 0.1f) {
				vector3f p = pp + (1-t)*(1-t)*(1-t)*(1-t)*p0 + t*t*t*t*p1;
				glVertex3fv(&p[0]);
			}
		}
		{
			vector3f p0(0,-spikerad,0), p1(-spikerad,0,0);
			float t=0.1; for (int i=1; i<10; i++, t+= 0.1f) {
				vector3f p = pp + (1-t)*(1-t)*(1-t)*(1-t)*p0 + t*t*t*t*p1;
				glVertex3fv(&p[0]);
			}
		}
		{
			vector3f p0(-spikerad,0,0), p1(0,spikerad,0);
			float t=0.1; for (int i=1; i<10; i++, t+= 0.1f) {
				vector3f p = pp + (1-t)*(1-t)*(1-t)*(1-t)*p0 + t*t*t*t*p1;
				glVertex3fv(&p[0]);
			}
		}
#endif
		glEnd();
		
		Render::State::UseProgram(0);
		Gui::Screen::LeaveOrtho();
		glDisable(GL_BLEND);

		
		//if (Render::AreShadersEnabled())


			// shaders get you pretty spots and things
			TerrainBody::Render(viewCoords, viewTransform);

#if 0
		else {
			// just the plain old disc

			// face the camera dammit
			vector3d zaxis = fpos.Normalized();
			vector3d xaxis = vector3d(0,1,0).Cross(zaxis).Normalized();
			vector3d yaxis = zaxis.Cross(xaxis);
			matrix4x4d rot = matrix4x4d::MakeInvRotMatrix(xaxis, yaxis, zaxis);
			glMultMatrixd(&rot[0]);

			glDisable(GL_LIGHTING);
			glDisable(GL_DEPTH_TEST);
			glBegin(GL_TRIANGLE_FAN);
			glColor4f(b*col[0],b*col[1],b*col[2],1);
			glVertex3f(0, 0, 0);
			for (float ang=0; ang<2*M_PI; ang+=0.1) {
				glVertex3f(float(rad*sin(ang)), float(rad*cos(ang)), 0);
			}
			glVertex3f(0, float(rad), 0);
			glEnd();
		}
#endif
	}

	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}
