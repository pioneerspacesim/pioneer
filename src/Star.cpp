// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Star.h"

#include "Pi.h"
#include "galaxy/SystemBody.h"
#include "galaxy/StarSystem.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"

Star::Star(SystemBody *sbody) :
	TerrainBody(sbody)
{
	InitStar();
}

Star::Star(const Json &jsonObj, Space *space) :
	TerrainBody(jsonObj, space)
{
	InitStar();
}

Star::~Star()
{
}

void Star::InitStar()
{
	// this one should be atmosphere radius when stars have atmosphere
	SetPhysRadius(GetMaxFeatureRadius());

	// this one is much larger because stars have halo effect
	// if star is wolf-rayet it gets a very large halo effect
	const SystemBody *sbody = GetSystemBody();
	const float wf = (sbody->GetType() < SystemBody::TYPE_STAR_S_BH && sbody->GetType() > SystemBody::TYPE_STAR_O_HYPER_GIANT) ? 100.0f : 1.0f;
	SetClipRadius(sbody->GetRadius() * 8 * wf);

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA;
	rsd.depthWrite = false;
	m_haloState = Pi::renderer->CreateRenderState(rsd);
}

void Star::BuildHaloBuffer(Graphics::Renderer *renderer, double rad)
{
	// build halo vertex buffer
	Random rand;
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE);
	const Color bright(StarSystem::starRealColors[GetSystemBody()->GetType()]);
	const Color dark(Color::BLANK);

	va.Add(vector3f(0.f), bright);
	for (float ang = 0; ang < 2 * M_PI; ang += 0.26183 + rand.Double(0, 0.4)) {
		va.Add(vector3f(sin(ang), cos(ang), 0), dark);
	}
	va.Add(vector3f(0.f, 1.f, 0.f), dark);

	//create buffer and upload data
	Graphics::VertexBufferDesc vbd;
	vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
	vbd.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
	vbd.attrib[1].semantic = Graphics::ATTRIB_DIFFUSE;
	vbd.attrib[1].format = Graphics::ATTRIB_FORMAT_UBYTE4;
	vbd.numVertices = va.GetNumVerts();
	vbd.usage = Graphics::BUFFER_USAGE_STATIC;
	m_haloBuffer.reset(renderer->CreateVertexBuffer(vbd));

	m_haloBuffer->Populate(va);
}

void Star::Render(Graphics::Renderer *renderer, const Camera *camera, const vector3d &viewCoords, const matrix4x4d &viewTransform)
{
	double rad = GetClipRadius();
	vector3d fpos = viewCoords;
	double len = fpos.Length();

	while (len > 1000.0f) {
		rad *= 0.25;
		fpos = 0.25 * fpos;
		len *= 0.25;
	}

	matrix4x4d trans = matrix4x4d::Identity();
	trans.Translate(float(fpos.x), float(fpos.y), float(fpos.z));

	// face the camera dammit
	vector3d zaxis = viewCoords.NormalizedSafe();
	vector3d xaxis = vector3d(0, 1, 0).Cross(zaxis).Normalized();
	vector3d yaxis = zaxis.Cross(xaxis);
	matrix4x4d rot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).Inverse();

	// Generate the halo if we don't have one
	if (!m_haloBuffer) {
		BuildHaloBuffer(renderer, rad);
	}
	// scale the halo by the new radius from it's unit size
	renderer->SetTransform(trans * matrix4x4d::ScaleMatrix(rad) * rot);
	//render star halo
	renderer->DrawBuffer(m_haloBuffer.get(), m_haloState, Graphics::vtxColorMaterial, Graphics::TRIANGLE_FAN);

	// the transform will be reset within TerrainBody::Render or it's subsequent calls
	TerrainBody::Render(renderer, camera, viewCoords, viewTransform);

	renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_STARS, 1);
}
