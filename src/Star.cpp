#include "Star.h"
#include "render/Render.h"
#include "render/Renderer.h"
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

void Star::Render(Renderer *renderer, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
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

	MTRand(rand);

	renderer->SetBlendMode(BLEND_ALPHA_ONE);

	//render star halo
	VertexArray va;
	const Color bright(col[0], col[1], col[2], 1.f);
	const Color dark(0.f, 0.f, 0.f, 0.f);
	va.position.push_back(vector3f(0.f, 0.f, 0.f));
	va.diffuse.push_back(bright);
	for (float ang=0; ang<2*M_PI; ang+=0.26183+rand.Double(0,0.4)) {
		va.position.push_back(vector3f(rad*sin(ang), rad*cos(ang), 0));
		va.diffuse.push_back(dark);
	}
	va.position.push_back(vector3f(0.f, rad, 0.f));
	va.diffuse.push_back(dark);
	renderer->DrawTriangles(&va, 0, TRIANGLE_FAN);

	Render::State::UseProgram(0);
	renderer->SetBlendMode(BLEND_SOLID);

	glPopMatrix();
	glEnable(GL_DEPTH_TEST);

	TerrainBody::Render(renderer, viewCoords, viewTransform);
}
