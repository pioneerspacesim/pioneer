// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Planet.h"

#include "Color.h"
#include "GeoSphere.h"
#include "galaxy/SystemBody.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"
#include "perlin.h"
#include "profiler/Profiler.h"

#ifdef _MSC_VER
#include "win32/WinMath.h"
#endif // _MSC_VER

using namespace Graphics;

static const Graphics::AttributeSet RING_VERTEX_ATTRIBS = Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0;

Planet::Planet(SystemBody *sbody) :
	TerrainBody(sbody)
{
	InitParams(sbody);
}

Planet::Planet(const Json &jsonObj, Space *space) :
	TerrainBody(jsonObj, space)
{
	const SystemBody *sbody = GetSystemBody();
	assert(sbody);
	InitParams(sbody);
}

void Planet::InitParams(const SystemBody *sbody)
{
	m_surfaceGravity_g = sbody->CalcSurfaceGravity();
	m_atmosphereRadius = sbody->GetAtmRadius() + sbody->GetRadius();

	SetPhysRadius(std::max(m_atmosphereRadius, GetMaxFeatureRadius() + 1000));
	// NB: Below abandoned due to docking problems with low altitude orbiting space stations
	// SetPhysRadius(std::max(m_atmosphereRadius, std::max(GetMaxFeatureRadius() * 2.0 + 2000, sbody->GetRadius() * 1.05)));
	if (sbody->HasRings()) {
		SetClipRadius(sbody->GetRadius() * sbody->GetRings().maxRadius.ToDouble());
	} else {
		SetClipRadius(GetPhysRadius());
	}
}

/*
 * dist = distance from centre
 * returns pressure in earth atmospheres
 * function is slightly different from the isothermal earth-based approximation used in shaders,
 * but it isn't visually noticeable.
 */
void Planet::GetAtmosphericState(double dist, double *outPressure, double *outDensity) const
{
	PROFILE_SCOPED()
#if 0
	static bool atmosphereTableShown = false;
	if (!atmosphereTableShown) {
		atmosphereTableShown = true;
		for (double h = -1000; h <= 100000; h = h+1000.0) {
			double p = 0.0, d = 0.0;
			GetAtmosphericState(h+this->GetSystemBody()->GetRadius(),&p,&d);
			Output("height(m): %f, pressure(hpa): %f, density: %f\n", h, p*101325.0/100.0, d);
		}
	}
#endif

	// This model has no atmosphere beyond the adiabatic limit
	if (dist >= m_atmosphereRadius) {
		*outDensity = 0.0;
		*outPressure = 0.0;
		return;
	}

	const SystemBody *sbody = GetSystemBody();
	const double height_h = (dist - sbody->GetRadius()); // height in m

	// height below zero should not occur
	if (height_h < 0.0) {
		*outPressure = sbody->GetAtmSurfacePressure();
		*outDensity = sbody->GetAtmSurfaceDensity();
		return;
	}

	*outPressure = sbody->GetAtmPressure(height_h);
	*outDensity = sbody->GetAtmDensity(height_h, *outPressure);
}

void Planet::GenerateRings(Graphics::Renderer *renderer)
{
	const SystemBody *sbody = GetSystemBody();

	Graphics::VertexArray ringVertices(RING_VERTEX_ATTRIBS);

	// generate the ring geometry
	const float inner = sbody->GetRings().minRadius.ToFloat();
	const float outer = sbody->GetRings().maxRadius.ToFloat();
	int segments = 200;
	for (int i = 0; i <= segments; ++i) {
		const float a = (2.0f * float(M_PI)) * (float(i) / float(segments));
		const float ca = cosf(a);
		const float sa = sinf(a);
		ringVertices.Add(vector3f(inner * sa, 0.0f, inner * ca), vector2f(float(i), 0.0f));
		ringVertices.Add(vector3f(outer * sa, 0.0f, outer * ca), vector2f(float(i), 1.0f));
	}

	// Upload vertex data to GPU
	m_ringMesh.reset(renderer->CreateMeshObjectFromArray(&ringVertices));

	// generate the ring texture
	// NOTE: texture width must be > 1 to avoid graphical glitches with Intel GMA 900 systems
	//       this is something to do with mipmapping (probably mipmap generation going wrong)
	//       (if the texture is generated without mipmaps then a 1xN texture works)
	const int RING_TEXTURE_WIDTH = 4;
	const int RING_TEXTURE_LENGTH = 256;
	std::unique_ptr<Color, FreeDeleter> buf(
		static_cast<Color *>(malloc(RING_TEXTURE_WIDTH * RING_TEXTURE_LENGTH * 4)));

	const float ringScale = (outer - inner) * sbody->GetRadius() / 1.5e7f;

	Random rng(GetSystemBody()->GetSeed() + 4609837);
	Color baseCol = sbody->GetRings().baseColor;
	double noiseOffset = 2048.0 * rng.Double();
	for (int i = 0; i < RING_TEXTURE_LENGTH; ++i) {
		const float alpha = (float(i) / float(RING_TEXTURE_LENGTH)) * ringScale;
		const float n = 0.25 +
			0.60 * noise(vector3d(5.0 * alpha, noiseOffset, 0.0)) +
			0.15 * noise(vector3d(10.0 * alpha, noiseOffset, 0.0));

		const float LOG_SCALE = 1.0f / sqrtf(sqrtf(log1p(1.0f)));
		const float v = LOG_SCALE * sqrtf(sqrtf(log1p(n)));

		Color color;
		color.r = v * baseCol.r;
		color.g = v * baseCol.g;
		color.b = v * baseCol.b;
		color.a = ((v * 0.25f) + 0.75f) * baseCol.a;

		Color *row = buf.get() + i * RING_TEXTURE_WIDTH;
		for (int j = 0; j < RING_TEXTURE_WIDTH; ++j) {
			row[j] = color;
		}
	}

	// first and last pixel are forced to zero, to give a slightly smoother ring edge
	{
		Color *row;
		row = buf.get();
		std::fill_n(row, RING_TEXTURE_WIDTH, Color::BLANK);
		row = buf.get() + (RING_TEXTURE_LENGTH - 1) * RING_TEXTURE_WIDTH;
		std::fill_n(row, RING_TEXTURE_WIDTH, Color::BLANK);
	}

	const vector3f texSize(RING_TEXTURE_WIDTH, RING_TEXTURE_LENGTH, 0.0f);
	const Graphics::TextureDescriptor texDesc(
		Graphics::TEXTURE_RGBA_8888, texSize, Graphics::LINEAR_REPEAT, true, true, true, 0, Graphics::TEXTURE_2D);

	m_ringTexture.Reset(renderer->CreateTexture(texDesc));
	m_ringTexture->Update(
		static_cast<void *>(buf.get()), texSize,
		Graphics::TEXTURE_RGBA_8888);

	Graphics::MaterialDescriptor desc;
	desc.lighting = true;
	desc.textures = 1;

	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA_PREMULT;
	rsd.cullMode = Graphics::CULL_NONE;
	rsd.primitiveType = Graphics::TRIANGLE_STRIP;

	m_ringMaterial.reset(renderer->CreateMaterial("planetrings", desc, rsd));
	m_ringMaterial->SetTexture("texture0"_hash, m_ringTexture.Get());
}

void Planet::DrawGasGiantRings(Renderer *renderer, const matrix4x4d &modelView)
{
	assert(GetSystemBody()->HasRings());

	if (!m_ringTexture)
		GenerateRings(renderer);

	renderer->SetTransform(matrix4x4f(modelView));
	renderer->DrawMesh(m_ringMesh.get(), m_ringMaterial.get());
}

void Planet::SubRender(Renderer *r, const matrix4x4d &viewTran, const vector3d &camPos)
{
	if (GetSystemBody()->HasRings()) {
		DrawGasGiantRings(r, viewTran);
	}
}
