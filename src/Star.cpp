// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Star.h"
#include "graphics/Graphics.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "gui/Gui.h"

using namespace Graphics;

Star::Star() : TerrainBody()
{
}

Star::Star(SystemBody *sbody): TerrainBody(sbody)
{
	m_hasDoubleFrame = false;
}

double Star::GetClipRadius() const
{

	const SystemBody *sbody = GetSystemBody();

	// if star is wolf-rayet it gets a very large halo effect
	const float wf = (sbody->type < SystemBody::TYPE_STAR_S_BH && sbody->type > SystemBody::TYPE_STAR_O_HYPER_GIANT) ? 100.0f : 1.0f;
	return sbody->GetRadius() * 72 * wf;
}

void Star::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	renderer->SetDepthTest(false);
	glPushMatrix();

	double radius = GetClipRadius();

	double rad = radius;
	vector3d fpos = viewCoords;
	double len = fpos.Length();

	while (len > 1000.0f) {
		rad *= 0.25;
		fpos = 0.25*fpos;
		len *= 0.25;
	}

	matrix4x4d trans = matrix4x4d::Identity();
	trans.Translate(float(fpos.x), float(fpos.y), float(fpos.z));

	// face the camera dammit
	vector3d zaxis = viewCoords.NormalizedSafe();
	vector3d xaxis = vector3d(0,1,0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();

	renderer->SetTransform(trans * rot);

	const float *col = StarSystem::starRealColors[GetSystemBody()->type];

	MTRand(rand);

	renderer->SetBlendMode(BLEND_ALPHA_ONE);

	//render star halos
	VertexArray va(ATTRIB_POSITION | ATTRIB_DIFFUSE); //inner
	VertexArray vb(ATTRIB_POSITION | ATTRIB_DIFFUSE); //outer
	const Color bright(col[0], col[1], col[2], 1.f);
	const Color dark(0.0f, 0.0f, 0.0f, 0.f);

	va.Add(vector3f(0.f), bright*0.775);  
	vb.Add(vector3f(0.f,0.f,0.01f), bright*0.5);

	float ang=0;
	const float size=4;
	for (ang=0; ang<2*M_PI; ang+=0.25+rand.Double(0,0.04)) {
		
		float xf = rad*sin(ang);
		float yf = rad*cos(ang);

		va.Add(vector3f(xf, yf, 0), dark);
		vb.Add(vector3f(xf*size, yf*size, 0.01f), dark);
	}
	float lf = rad*cos(2*M_PI-ang);
	va.Add(vector3f(0.f, lf, 0.f), dark);
	vb.Add(vector3f(0.f, lf*size, 0.01f), dark);

	renderer->DrawTriangles(&vb, Graphics::vtxColorMaterial, TRIANGLE_FAN);
	renderer->DrawTriangles(&va, Graphics::vtxColorMaterial, TRIANGLE_FAN);
	renderer->SetBlendMode(BLEND_SOLID);

	glPopMatrix();
	renderer->SetDepthTest(true);

	TerrainBody::Render(renderer, camera, viewCoords, viewTransform);
}
