// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GasGiantJobs.h"

#include "GasGiant.h"
#include "Pi.h"
#include "RefCounted.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "graphics/opengl/GenGasGiantColourMaterial.h"
#include "perlin.h"
#include "terrain/TerrainNoise.h"
#include "vcacheopt/vcacheopt.h"
#include <algorithm>
#include <deque>

namespace GasGiantJobs
{
	static const vector3d s_patchFaces[NUM_PATCHES][4] = {
		{ p5, p1, p4, p8 }, // +x
		{ p2, p6, p7, p3 }, // -x

		{ p2, p1, p5, p6 }, // +y
		{ p7, p8, p4, p3 }, // -y

		{ p6, p5, p8, p7 }, // +z - NB: these are actually reversed!
		{ p1, p2, p3, p4 }	// -z
	};
	const vector3d& GetPatchFaces(const Uint32 patch, const Uint32 face) {
		return s_patchFaces[patch][face];
	}
	static std::map<int, SDLSurfacePtr> s_gasGiantTextures;
	static void InitGasGiantCPUTextures() {
		if (s_gasGiantTextures.empty())
		{
			s_gasGiantTextures[Graphics::OGL::GEN_JUPITER_TEXTURE] = LoadSurfaceFromFile("textures/gasgiants/jupiterramp.png");
			s_gasGiantTextures[Graphics::OGL::GEN_JUPITER_TEXTURE] = LoadSurfaceFromFile("textures/gasgiants/jupiterramp.png");
			s_gasGiantTextures[Graphics::OGL::GEN_SATURN_TEXTURE] = LoadSurfaceFromFile("textures/gasgiants/saturnramp.png");
			s_gasGiantTextures[Graphics::OGL::GEN_SATURN2_TEXTURE] = LoadSurfaceFromFile("textures/gasgiants/saturn2ramp.png");
			s_gasGiantTextures[Graphics::OGL::GEN_NEPTUNE_TEXTURE] = LoadSurfaceFromFile("textures/gasgiants/neptuneramp.png");
			s_gasGiantTextures[Graphics::OGL::GEN_NEPTUNE2_TEXTURE] = LoadSurfaceFromFile("textures/gasgiants/neptune2ramp.png");
			s_gasGiantTextures[Graphics::OGL::GEN_URANUS_TEXTURE] = LoadSurfaceFromFile("textures/gasgiants/uranusramp.png");
		}
	}

	inline float dot(const Color4f &a, const Color4f &b) { return a.r * b.r + a.g * b.g + a.b * b.b; }

	// HueAdjustment function based on this StackOverflow answer
	// http://stackoverflow.com/questions/9234724/how-to-change-hue-of-a-texture-with-glsl/9234854#9234854
	Color4f HueShift(Color4f color, const float hueAdjust)
	{
		static const Color4f kRGBToYPrime = Color4f (0.299, 0.587, 0.114, 0.0);
		static const Color4f kRGBToI = Color4f (0.596, -0.275, -0.321, 0.0);
		static const Color4f kRGBToQ = Color4f (0.212, -0.523, 0.311, 0.0);

		static const Color4f kYIQToR = Color4f (1.0, 0.956, 0.621, 0.0);
		static const Color4f kYIQToG = Color4f (1.0, -0.272, -0.647, 0.0);
		static const Color4f kYIQToB = Color4f (1.0, -1.107, 1.704, 0.0);

		// Convert to YIQ
		float YPrime = dot (color, kRGBToYPrime);
		float I = dot (color, kRGBToI);
		float Q = dot (color, kRGBToQ);

		// Calculate the hue and chroma
		float hue = atan2 (Q, I);
		float chroma = sqrt (I * I + Q * Q);

		// Make the user's adjustments
		hue += hueAdjust;

		// Convert back to YIQ
		Q = chroma * sin (hue);
		I = chroma * cos (hue);

		// Convert back to RGB
		Color4f yIQ = Color4f (YPrime, I, Q, 0.0);
		color.r = dot (yIQ, kYIQToR);
		color.g = dot (yIQ, kYIQToG);
		color.b = dot (yIQ, kYIQToB);

		// the result
		return color;
	}

	inline Color4f texture(const SDLSurfacePtr &pTex, const vector2d &uv)
	{
		Color4f ret(Color4f::WHITE);

		const int uvx = Clamp((int)std::floor(uv.x * pTex->w), 0, pTex->w-1);
		const int uvy = Clamp((int)std::floor(uv.y * pTex->h), 0, pTex->h-1);

		ret.r = (static_cast<unsigned char *>(pTex->pixels)[uvx + (uvy * pTex->pitch) + 0]);
		ret.g = (static_cast<unsigned char *>(pTex->pixels)[uvx + (uvy * pTex->pitch) + 1]);
		ret.b = (static_cast<unsigned char *>(pTex->pixels)[uvx + (uvy * pTex->pitch) + 2]);
		ret.a = 1.0f;
		return ret;
	}

	static const int FBM_OCTAVES = 8; // hardcoded in the shader too, probably needs to be more flexible but... meh
	inline Color4f GetColour(const vector3d &p,
		const SDLSurfacePtr &texture2,
		float fracStep,
		const vector3d &frequency)
	{
		float n1 = TerrainNoise::octavenoise(FBM_OCTAVES, frequency.x, 0.5, p * 4.0);
		float n2 = TerrainNoise::octavenoise(FBM_OCTAVES, frequency.x, 0.5, p * 3.14159);
		Color4f color = Color4f(texture(texture2, vector2d(n1 * 0.075, (n2 * 0.075) + ((p.y + 1.0) * 0.5))));
		return color;
	}

	// in patch surface coords, [0,1]
	inline vector3d GetSpherePoint(const double x, const double y, const vector3d *corners)
	{
		return (corners[0] + x * (1.0 - y) * (corners[1] - corners[0]) + x * y * (corners[2] - corners[0]) + (1.0 - x) * y * (corners[3] - corners[0])).Normalized();
	}

	void InstantTextureGenerator(const double fracStep, const Terrain *pTerrain, const Uint32 GGQuality, const float hueAdjust, Graphics::Texture *texOut)
	{
		InitGasGiantCPUTextures();
		const int texSize = texOut->GetDescriptor().dataSize.x;

		SDLSurfacePtr im = s_gasGiantTextures[GGQuality];
		vector3d freq;
		for (Uint32 i = 0; i < 3; i++) {
			freq[i] = float(pTerrain->GetFracDef(i).frequency);
		}

		Graphics::TextureCubeData tcd;
		std::unique_ptr<Color[]> bufs[NUM_PATCHES];
		for (Uint32 i = 0; i < NUM_PATCHES; i++) {
			Color *colors = new Color[(texSize * texSize)];
			for (Uint32 v = 0; v < texSize; v++) {
				for (Uint32 u = 0; u < texSize; u++) {
					// where in this row & colum are we now.
					const double ustep = double(u) * fracStep;
					const double vstep = double(v) * fracStep;

					// get point on the surface of the sphere
					const vector3d p = GetSpherePoint(ustep, vstep, &GetPatchFaces(i, 0));
					// get colour using `p`
					const Color4f colour = HueShift(GetColour(p, im, fracStep, freq), hueAdjust);

					// convert to ubyte and store
					Color *col = colors + (u + (v * texSize));
					(*col) = colour;
				}
			}
			bufs[i].reset(colors);
		}

		// update with buffer from above
		tcd.posX = bufs[0].get();
		tcd.negX = bufs[1].get();
		tcd.posY = bufs[2].get();
		tcd.negY = bufs[3].get();
		tcd.posZ = bufs[4].get();
		tcd.negZ = bufs[5].get();
		texOut->Update(tcd, texOut->GetDescriptor().dataSize, Graphics::TEXTURE_RGBA_8888);
	}

	STextureFaceRequest::STextureFaceRequest(const SystemPath &sysPath_, const Sint32 face_, const Sint32 uvDIMs_, const Terrain *pTerrain_, const Uint32 GGQuality, const float hueAdjust_) :
		corners(&GetPatchFaces(face_, 0)),
		sysPath(sysPath_),
		face(face_),
		uvDIMs(uvDIMs_),
		pTerrain(pTerrain_),
		ggQuality((Graphics::OGL::GasGiantQuality)GGQuality),
		hueAdjust(hueAdjust_)
	{
		InitGasGiantCPUTextures();
		colors = new Color[NumTexels()];
	}

	// RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	// Use only data local to this object
	void STextureFaceRequest::OnRun()
	{
		PROFILE_SCOPED()
		//MsgTimer timey;
		SDLSurfacePtr im = s_gasGiantTextures[ggQuality];
		vector3d freq;
		for (Uint32 i = 0; i < 3; i++) {
			freq[i] = float(pTerrain->GetFracDef(i).frequency);
		}

		assert(corners != nullptr);
		double fracStep = 1.0 / double(UVDims() - 1);
		for (Sint32 v = 0; v < UVDims(); v++) {
			for (Sint32 u = 0; u < UVDims(); u++) {
				// where in this row & colum are we now.
				const double ustep = double(u) * fracStep;
				const double vstep = double(v) * fracStep;

				// get point on the surface of the sphere
				const vector3d p = GetSpherePoint(ustep, vstep, corners);
				// get colour using `p`
				const Color4f colour = HueShift(GetColour(p, im, fracStep, freq), hueAdjust);

				// convert and store
				Color *col = colors + (u + (v * UVDims()));
				(*col) = colour;
			}
		}
	}

	// ********************************************************************************
	// Overloaded PureJob class to handle generating the mesh for each patch
	// ********************************************************************************
	SingleTextureFaceJob::~SingleTextureFaceJob()
	{
		PROFILE_SCOPED()
		if (mpResults) {
			mpResults->OnCancel();
			delete mpResults;
			mpResults = nullptr;
		}
	}

	void SingleTextureFaceJob::OnRun() // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	{
		PROFILE_SCOPED()
		mData->OnRun();

		// add this patches data
		STextureFaceResult *sr = new STextureFaceResult(mData->Face());
		sr->addResult(mData->Colors(), mData->UVDims());

		// store the result
		mpResults = sr;
	}

	void SingleTextureFaceJob::OnFinish() // runs in primary thread of the context
	{
		PROFILE_SCOPED()
		GasGiant::OnAddTextureFaceResult(mData->SysPath(), mpResults);
		mpResults = nullptr;
	}

	// ********************************************************************************
	GenFaceQuad::GenFaceQuad(Graphics::Renderer *r, const vector2f &size, Graphics::RenderState *state, const Uint32 GGQuality)
	{
		PROFILE_SCOPED()
		assert(state);
		m_renderState = state;

		Graphics::MaterialDescriptor desc;
		desc.effect = Graphics::EFFECT_GEN_GASGIANT_TEXTURE;
		desc.quality = GGQuality;
		desc.textures = 3;
		m_material.reset(r->CreateMaterial(desc));

		// setup noise textures
		m_material->texture0 = Graphics::TextureBuilder::Raw("textures/permTexture.png").GetOrCreateTexture(Pi::renderer, "noise");
		m_material->texture1 = Graphics::TextureBuilder::Raw("textures/gradTexture.png").GetOrCreateTexture(Pi::renderer, "noise");

		// pick the correct colour basis texture for the planet
		switch (0x0000FFFF & GGQuality) {
		case Graphics::OGL::GEN_JUPITER_TEXTURE:
			m_material->texture2 = Graphics::TextureBuilder::Raw("textures/gasgiants/jupiterramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		case Graphics::OGL::GEN_SATURN_TEXTURE:
			m_material->texture2 = Graphics::TextureBuilder::Raw("textures/gasgiants/saturnramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		case Graphics::OGL::GEN_SATURN2_TEXTURE:
			m_material->texture2 = Graphics::TextureBuilder::Raw("textures/gasgiants/saturn2ramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		case Graphics::OGL::GEN_NEPTUNE_TEXTURE:
			m_material->texture2 = Graphics::TextureBuilder::Raw("textures/gasgiants/neptuneramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		case Graphics::OGL::GEN_NEPTUNE2_TEXTURE:
			m_material->texture2 = Graphics::TextureBuilder::Raw("textures/gasgiants/neptune2ramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		case Graphics::OGL::GEN_URANUS_TEXTURE:
			m_material->texture2 = Graphics::TextureBuilder::Raw("textures/gasgiants/uranusramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		}

		// these might need to be reversed
		const vector2f &texSize(size);

		Graphics::VertexArray vertices(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

		vertices.Add(vector3f(0.0f, 0.0f, 0.0f), vector2f(0.0f, texSize.y));
		vertices.Add(vector3f(0.0f, size.y, 0.0f), vector2f(0.0f, 0.0f));
		vertices.Add(vector3f(size.x, 0.0f, 0.0f), vector2f(texSize.x, texSize.y));
		vertices.Add(vector3f(size.x, size.y, 0.0f), vector2f(texSize.x, 0.0f));

		//Create vtx & index buffers and copy data
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[1].semantic = Graphics::ATTRIB_UV0;
		vbd.attrib[1].format = Graphics::ATTRIB_FORMAT_FLOAT2;
		vbd.numVertices = vertices.GetNumVerts();
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;
		m_vertexBuffer.reset(r->CreateVertexBuffer(vbd));
		m_vertexBuffer->Populate(vertices);
	}

	void GenFaceQuad::Draw(Graphics::Renderer *r)
	{
		PROFILE_SCOPED()
		r->DrawBuffer(m_vertexBuffer.get(), m_renderState, m_material.get(), Graphics::TRIANGLE_STRIP);
	}

	// ********************************************************************************
	SGPUGenRequest::SGPUGenRequest(const SystemPath &sysPath_, const Sint32 uvDIMs_, Terrain *pTerrain_, const float planetRadius_, const float hueAdjust_, GenFaceQuad *pQuad_, Graphics::Texture *pTex_) :
		m_texture(pTex_),
		sysPath(sysPath_),
		uvDIMs(uvDIMs_),
		pTerrain(pTerrain_),
		planetRadius(planetRadius_),
		hueAdjust(hueAdjust_),
		pQuad(pQuad_)
	{
		PROFILE_SCOPED()
		assert(m_texture.Valid());
	}

	void SGPUGenRequest::SetupMaterialParams(const int face)
	{
		PROFILE_SCOPED()
		m_specialParams.v = &(GetPatchFaces(face, 0));
		m_specialParams.fracStep = 1.0f / float(uvDIMs);
		m_specialParams.planetRadius = planetRadius;
		m_specialParams.time = 0.0f;

		for (Uint32 i = 0; i < 3; i++) {
			m_specialParams.frequency[i] = float(pTerrain->GetFracDef(i).frequency);
		}

		m_specialParams.hueAdjust = hueAdjust;

		pQuad->GetMaterial()->specialParameter0 = &m_specialParams;
	}

	// ********************************************************************************
	void SGPUGenResult::addResult(Graphics::Texture *t_, Sint32 uvDims_)
	{
		PROFILE_SCOPED()
		mData = SGPUGenData(t_, uvDims_);
	}

	void SGPUGenResult::OnCancel()
	{
		if (mData.texture) {
			mData.texture.Reset();
		}
	}

	// ********************************************************************************
	// Overloaded JobGPU class to handle generating the mesh for each patch
	// ********************************************************************************
	SingleGPUGenJob::SingleGPUGenJob(SGPUGenRequest *data) :
		mData(data),
		mpResults(nullptr)
	{ /* empty */
	}
	SingleGPUGenJob::~SingleGPUGenJob()
	{
		PROFILE_SCOPED()
		if (mpResults) {
			mpResults->OnCancel();
			delete mpResults;
			mpResults = nullptr;
		}
	}

	void SingleGPUGenJob::OnRun() // Runs in the main thread, may trash the GPU state
	{
		PROFILE_SCOPED()

		Graphics::Renderer::StateTicket ticket(Pi::renderer);

		// enter ortho
		Pi::renderer->SetViewport({ 0, 0, mData->UVDims(), mData->UVDims() });
		Pi::renderer->SetOrthographicProjection(0, mData->UVDims(), mData->UVDims(), 0, -1, 1);
		Pi::renderer->SetTransform(matrix4x4f::Identity());

		GasGiant::BeginRenderTarget();
		for (Uint32 iFace = 0; iFace < NUM_PATCHES; iFace++) {
			// render the scene
			GasGiant::SetRenderTargetCubemap(iFace, mData->Texture());
			Pi::renderer->BeginFrame();

			// draw to the texture here
			mData->SetupMaterialParams(iFace);
			mData->Quad()->Draw(Pi::renderer);

			Pi::renderer->EndFrame();
			GasGiant::SetRenderTargetCubemap(iFace, nullptr);
		}
		GasGiant::EndRenderTarget();

		// add this patches data
		SGPUGenResult *sr = new SGPUGenResult();
		sr->addResult(mData->Texture(), mData->UVDims());

		// store the result
		mpResults = sr;

		// leave ortho when ticket is destroyed
	}

	void SingleGPUGenJob::OnFinish() // runs in primary thread of the context
	{
		PROFILE_SCOPED()
		GasGiant::OnAddGPUGenResult(mData->SysPath(), mpResults);
		mpResults = nullptr;
	}
} // namespace GasGiantJobs
