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
#include "perlin.h"

#include <algorithm>
#include <cstddef>
#include <deque>

namespace GasGiantJobs {
	static const vector3d s_patchFaces[NUM_PATCHES][4] = {
		{ p5, p1, p4, p8 }, // +x
		{ p2, p6, p7, p3 }, // -x

		{ p2, p1, p5, p6 }, // +y
		{ p7, p8, p4, p3 }, // -y

		{ p6, p5, p8, p7 }, // +z - NB: these are actually reversed!
		{ p1, p2, p3, p4 }	// -z
	};
	const vector3d &GetPatchFaces(const Uint32 patch, const Uint32 face) { return s_patchFaces[patch][face]; }

	STextureFaceRequest::STextureFaceRequest(const vector3d *v_, const SystemPath &sysPath_, const Sint32 face_, const Sint32 uvDIMs_, Terrain *pTerrain_) :
		corners(v_),
		sysPath(sysPath_),
		face(face_),
		uvDIMs(uvDIMs_),
		pTerrain(pTerrain_)
	{
		colors = new Color[NumTexels()];
	}

	// RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
	// Use only data local to this object
	void STextureFaceRequest::OnRun()
	{
		PROFILE_SCOPED()

		assert(corners != nullptr);
		double fracStep = 1.0 / double(UVDims() - 1);
		for (Sint32 v = 0; v < UVDims(); v++) {
			for (Sint32 u = 0; u < UVDims(); u++) {
				// where in this row & column are we now.
				const double ustep = double(u) * fracStep;
				const double vstep = double(v) * fracStep;

				// get point on the surface of the sphere
				const vector3d p = GetSpherePoint(ustep, vstep);
				// get colour using `p`
				const vector3d colour = pTerrain->GetColor(p, 0.0, p);

				// convert to ubyte and store
				Color *col = colors + (u + (v * UVDims()));
				col[0].r = Uint8(colour.x * 255.0);
				col[0].g = Uint8(colour.y * 255.0);
				col[0].b = Uint8(colour.z * 255.0);
				col[0].a = 255;
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

	struct GenFaceDataBlock {
		alignas(16) vector3f v0;
		alignas(16) vector3f v1;
		alignas(16) vector3f v2;
		alignas(16) vector3f v3;
		float frequency;
		float fracStep;
		float hueAdjust;
		float time;
	};
	static_assert(sizeof(GenFaceDataBlock) == 80);
	static_assert(offsetof(GenFaceDataBlock, frequency) == 60);

	// ********************************************************************************
	GenFaceQuad::GenFaceQuad(Graphics::Renderer *r, const vector2f &size, const Uint32 GGQuality)
	{
		PROFILE_SCOPED()

		Graphics::MaterialDescriptor desc;
		desc.quality = Graphics::MaterialQuality::HAS_OCTAVES | (GGQuality & 0xFFFF0000);
		desc.textures = 3;

		Graphics::RenderStateDesc rsd;
		rsd.depthTest = false;
		rsd.depthWrite = false;
		rsd.blendMode = Graphics::BLEND_ALPHA;
		rsd.primitiveType = Graphics::TRIANGLE_STRIP;
		m_material.reset(r->CreateMaterial("gen_gas_giant_colour", desc, rsd));

		// setup noise textures
		m_material->SetTexture("permTexture"_hash,
			Graphics::TextureBuilder::Raw("textures/permTexture.png").GetOrCreateTexture(Pi::renderer, "noise"));
		m_material->SetTexture("gradTexture"_hash,
			Graphics::TextureBuilder::Raw("textures/gradTexture.png").GetOrCreateTexture(Pi::renderer, "noise"));

		Graphics::Texture *rampTexture = nullptr;
		// pick the correct colour basis texture for the planet
		switch (0x0000FFFF & GGQuality) {
		case GasGiantTexture::GEN_JUPITER_TEXTURE:
			rampTexture = Graphics::TextureBuilder::Raw("textures/gasgiants/jupiterramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		case GasGiantTexture::GEN_SATURN_TEXTURE:
			rampTexture = Graphics::TextureBuilder::Raw("textures/gasgiants/saturnramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		case GasGiantTexture::GEN_SATURN2_TEXTURE:
			rampTexture = Graphics::TextureBuilder::Raw("textures/gasgiants/saturn2ramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		case GasGiantTexture::GEN_NEPTUNE_TEXTURE:
			rampTexture = Graphics::TextureBuilder::Raw("textures/gasgiants/neptuneramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		case GasGiantTexture::GEN_NEPTUNE2_TEXTURE:
			rampTexture = Graphics::TextureBuilder::Raw("textures/gasgiants/neptune2ramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		case GasGiantTexture::GEN_URANUS_TEXTURE:
			rampTexture = Graphics::TextureBuilder::Raw("textures/gasgiants/uranusramp.png").GetOrCreateTexture(Pi::renderer, "gasgiant");
			break;
		}

		m_material->SetTexture("rampTexture"_hash, rampTexture);

		// these might need to be reversed
		const vector2f &texSize(size);

		Graphics::VertexArray vertices(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

		vertices.Add(vector3f(0.0f, 0.0f, 0.0f), vector2f(0.0f, texSize.y));
		vertices.Add(vector3f(0.0f, size.y, 0.0f), vector2f(0.0f, 0.0f));
		vertices.Add(vector3f(size.x, 0.0f, 0.0f), vector2f(texSize.x, texSize.y));
		vertices.Add(vector3f(size.x, size.y, 0.0f), vector2f(texSize.x, 0.0f));

		//Create vtx  buffer and copy data
		m_quadMesh.reset(r->CreateMeshObjectFromArray(&vertices));
	}

	void GenFaceQuad::Draw(Graphics::Renderer *r)
	{
		PROFILE_SCOPED()
		r->DrawMesh(m_quadMesh.get(), m_material.get());
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

		GenFaceDataBlock dataBlock{};

		dataBlock.v0 = vector3f(GetPatchFaces(face, 0));
		dataBlock.v1 = vector3f(GetPatchFaces(face, 1));
		dataBlock.v2 = vector3f(GetPatchFaces(face, 2));
		dataBlock.v3 = vector3f(GetPatchFaces(face, 3));

		dataBlock.frequency = float(pTerrain->GetFracDef(2).frequency);
		dataBlock.fracStep = 1.0f / float(uvDIMs);
		dataBlock.hueAdjust = hueAdjust;
		dataBlock.time = 0;

		pQuad->GetMaterial()->SetBufferDynamic("GenColorData"_hash, &dataBlock);
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
		Pi::renderer->SetOrthographicProjection(0, mData->UVDims(), mData->UVDims(), 0, -1, 1);
		Pi::renderer->SetTransform(matrix4x4f::Identity());

		GasGiant::BeginRenderTarget();
		for (Uint32 iFace = 0; iFace < NUM_PATCHES; iFace++) {
			// render the scene
			GasGiant::SetRenderTargetCubemap(iFace, mData->Texture());
			Pi::renderer->SetViewport({ 0, 0, mData->UVDims(), mData->UVDims() });
			Pi::renderer->ClearScreen();

			// draw to the texture here
			mData->SetupMaterialParams(iFace);
			mData->Quad()->Draw(Pi::renderer);

			// force the texture to be rendered before we modify the render target
			// FIXME: use different render targets for each cubemap face
			Pi::renderer->FlushCommandBuffers();
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
