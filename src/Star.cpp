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

double Star::GetClipRadius() const
{

	const SBody *sbody = GetSBody();

	// if star is wolf-rayet it gets a very large halo effect
	const float wf = (sbody->type < SBody::TYPE_STAR_S_BH && sbody->type > SBody::TYPE_STAR_O_HYPER_GIANT) ? 100.0f : 1.0f;
	return sbody->GetRadius() * 8 * wf;
}

void Star::Render(const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glPushMatrix();

	Render::State::UseProgram(0);

	double radius = GetClipRadius();
	
	double rad = radius;
	vector3d fpos = viewCoords;
	double len = fpos.Length();

	while (len > 1000.0f) {
		rad *= 0.25;
		fpos = 0.25*fpos;
		len *= 0.25;
	}

	glTranslatef(float(fpos.x), float(fpos.y), float(fpos.z));
	
	// face the camera dammit
	vector3d zaxis = viewCoords.NormalizedSafe();
	vector3d xaxis = vector3d(0,1,0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();
	glMultMatrixd(&rot[0]);

	const float *col = StarSystem::starRealColors[GetSBody()->type];
	const float b = 1.0f;

	MTRand(rand);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	
	glEnable(GL_BLEND);
	glBegin(GL_TRIANGLE_FAN);
	glColor4f(col[0], col[1], col[2], 1);
	glVertex3f(0,0,0);
	glColor4f(0,0,0,0);
	for (float ang=0; ang<2*M_PI; ang+=0.26183+rand.Double(0,0.4)) {
		glVertex3f(rad*sin(ang), rad*cos(ang), 0);
	}
	glVertex3f(0, rad, 0);
	glEnd();
	glDisable(GL_BLEND);
	
	Render::State::UseProgram(Render::simpleShader);
	glEnable(GL_BLEND);
	glColor4f(b*col[0],b*col[1],b*col[2],1);
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0,0,0);
	glColor4f(0,0,0,0);
	glEnd();
	
	Render::State::UseProgram(0);
	glDisable(GL_BLEND);

	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	TerrainBody::Render(viewCoords, viewTransform);
}
