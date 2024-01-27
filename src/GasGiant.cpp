// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GasGiant.h"

#include "FileSystem.h"
#include "Game.h"
#include "GameConfig.h"
#include "Pi.h"
#include "galaxy/AtmosphereParameters.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "perlin.h"
#include "utils.h"
#include "vcacheopt/vcacheopt.h"

RefCountedPtr<GasPatchContext> GasGiant::s_patchContext;
Graphics::RenderTarget *GasGiant::s_renderTarget;

namespace {
	static Uint32 s_texture_size_small = 16;
	static Uint32 s_texture_size_cpu[5];
	static Uint32 s_texture_size_gpu[5];
	static Uint32 s_noiseOctaves[5];
	static float s_initialCPUDelayTime = 60.0f; // (perhaps) 60 seconds seems like a reasonable default
	static float s_initialGPUDelayTime = 5.0f;	// (perhaps) 5 seconds seems like a reasonable default
	static std::vector<GasGiant *> s_allGasGiants;

	static const std::string GGJupiter("GGJupiter");
	static const std::string GGNeptune("GGNeptune");
	static const std::string GGNeptune2("GGNeptune2");
	static const std::string GGSaturn("GGSaturn");
	static const std::string GGSaturn2("GGSaturn2");
	static const std::string GGUranus("GGUranus");

	bool SplitData(const std::string &spec, Uint32 &cpuOut, Uint32 &gpuOut, Uint32 &octavesOut)
	{
		static const std::string delim(",");

		enum dataEntries {
			eCPU = 0,
			eGPU,
			eOCTAVES
		};

		size_t i = 0, start = 0, end = 0;
		while (end != std::string::npos) {
			// get to the first non-delim char
			start = spec.find_first_not_of(delim, end);

			// read the end, no more to do
			if (start == std::string::npos)
				break;

			// find the end - next delim or end of string
			end = spec.find_first_of(delim, start);

			// extract the fragment and remember it
			switch (i) {
			case eCPU:
				cpuOut = ceil_pow2(Clamp(atoi(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start).c_str()), 64, 4096));
				break;
			case eGPU:
				gpuOut = ceil_pow2(Clamp(atoi(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start).c_str()), 64, 4096));
				break;
			case eOCTAVES:
				octavesOut = Clamp(atoi(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start).c_str()), 1, 16);
				break;
			default:
				assert(false);
				break;
			}
			i++;
		}

		return i == 4;
	}
} // namespace

class GasPatchContext : public RefCounted {
public:
#pragma pack(push, 4)
	struct VBOVertex {
		vector3f pos;
		vector3f norm;
	};
#pragma pack(pop)

	int edgeLen;

	inline int IDX_VBO_COUNT_ALL_IDX() const { return ((edgeLen - 1) * (edgeLen - 1)) * 2 * 3; }

	inline int NUMVERTICES() const { return edgeLen * edgeLen; }

	double frac;

	std::unique_ptr<Uint32[]> indices;
	RefCountedPtr<Graphics::IndexBuffer> indexBuffer;

	GasPatchContext(const int _edgeLen) :
		edgeLen(_edgeLen)
	{
		Init();
	}

	~GasPatchContext()
	{
		Cleanup();
	}

	void Refresh()
	{
		Cleanup();
		Init();
	}

	void Cleanup()
	{
		indices.reset();
	}

	int GetIndices(std::vector<Uint32> &pl)
	{
		// calculate how many tri's there are
		const int tri_count = IDX_VBO_COUNT_ALL_IDX() / 3;

		// pre-allocate enough space
		pl.reserve(IDX_VBO_COUNT_ALL_IDX());

		// add all of the middle indices
		for (int i = 0; i < IDX_VBO_COUNT_ALL_IDX(); ++i) {
			pl.push_back(indices[i]);
		}

		return tri_count;
	}

	void Init()
	{
		PROFILE_SCOPED()
		frac = 1.0 / double(edgeLen - 1);

		// also want vtx indices for tris not touching edge of patch
		indices.reset(new Uint32[IDX_VBO_COUNT_ALL_IDX()]);
		Uint32 *idx = indices.get();
		for (int x = 0; x < edgeLen - 1; x++) {
			for (int y = 0; y < edgeLen - 1; y++) {
				idx[0] = x + edgeLen * y;
				idx[1] = x + 1 + edgeLen * y;
				idx[2] = x + edgeLen * (y + 1);
				idx += 3;

				idx[0] = x + 1 + edgeLen * y;
				idx[1] = x + 1 + edgeLen * (y + 1);
				idx[2] = x + edgeLen * (y + 1);
				idx += 3;
			}
		}

		// these will hold the optimised indices
		std::vector<Uint32> pl_short;

		// populate the N indices lists from the arrays built during InitTerrainIndices()
		// iterate over each index list and optimize it
		Uint32 tri_count = GetIndices(pl_short);
		VertexCacheOptimizerUInt vco;
#ifndef NDEBUG
		VertexCacheOptimizerUInt::Result res = vco.Optimize(&pl_short[0], tri_count);
		assert(0 == res);
#else
		vco.Optimize(&pl_short[0], tri_count);
#endif

		//create buffer & copy
		indexBuffer.Reset(Pi::renderer->CreateIndexBuffer(pl_short.size(), Graphics::BUFFER_USAGE_STATIC));
		Uint32 *idxPtr = indexBuffer->Map(Graphics::BUFFER_MAP_WRITE);
		for (Uint32 j = 0; j < pl_short.size(); j++) {
			idxPtr[j] = pl_short[j];
		}
		indexBuffer->Unmap();

		if (indices) {
			indices.reset();
		}
	}
};

class GasPatch {
public:
	RefCountedPtr<GasPatchContext> ctx;
	vector3d v[4];
	std::unique_ptr<Graphics::MeshObject> m_patchMesh;
	GasGiant *gasSphere;
	vector3d clipCentroid;
	double clipRadius;

	GasPatch(const RefCountedPtr<GasPatchContext> &_ctx, GasGiant *gs, vector3d v0, vector3d v1, vector3d v2, vector3d v3) :
		ctx(_ctx),
		gasSphere(gs),
		clipCentroid(((v0 + v1 + v2 + v3) * 0.25).Normalized()),
		clipRadius(0.0)
	{
		PROFILE_SCOPED()
		v[0] = v0;
		v[1] = v1;
		v[2] = v2;
		v[3] = v3;
		for (int i = 0; i < 4; i++) {
			clipRadius = std::max(clipRadius, (v[i] - clipCentroid).Length());
		}

		UpdateVBOs();
	}

	~GasPatch() {}

	/* in patch surface coords, [0,1] */
	vector3d GetSpherePoint(const double x, const double y) const
	{
		return (v[0] + x * (1.0 - y) * (v[1] - v[0]) + x * y * (v[2] - v[0]) + (1.0 - x) * y * (v[3] - v[0])).Normalized();
	}

	void UpdateVBOs()
	{
		PROFILE_SCOPED()
		//create buffer and upload data
		auto vbd = Graphics::VertexBufferDesc::FromAttribSet(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL);
		vbd.numVertices = ctx->NUMVERTICES();
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;
		Graphics::VertexBuffer *vtxBuffer = Pi::renderer->CreateVertexBuffer(vbd);

		GasPatchContext::VBOVertex *vtxPtr = vtxBuffer->Map<GasPatchContext::VBOVertex>(Graphics::BUFFER_MAP_WRITE);
		assert(vtxBuffer->GetDesc().stride == sizeof(GasPatchContext::VBOVertex));

		const Sint32 edgeLen = ctx->edgeLen;
		const double frac = ctx->frac;
		for (Sint32 y = 0; y < edgeLen; y++) {
			for (Sint32 x = 0; x < edgeLen; x++) {
				const vector3d p = GetSpherePoint(x * frac, y * frac);
				const vector3d pSubCentroid = p - clipCentroid;
				clipRadius = std::max(clipRadius, p.Length());
				vtxPtr->pos = vector3f(pSubCentroid);
				vtxPtr->norm = vector3f(p);

				++vtxPtr; // next vertex
			}
		}
		vtxBuffer->Unmap();

		m_patchMesh.reset(Pi::renderer->CreateMeshObject(vtxBuffer, ctx->indexBuffer.Get()));
	}

	void Render(Graphics::Renderer *renderer, const vector3d &campos, const matrix4x4d &modelView, const Graphics::Frustum &frustum)
	{
		if (!frustum.TestPoint(clipCentroid, clipRadius))
			return;

		const vector3d relpos = clipCentroid - campos;
		renderer->SetTransform(matrix4x4f(modelView * matrix4x4d::Translation(relpos)));

		Pi::statSceneTris += 2 * (ctx->edgeLen - 1) * (ctx->edgeLen - 1);
		++Pi::statNumPatches;

		renderer->DrawMesh(m_patchMesh.get(), gasSphere->GetSurfaceMaterial().Get());
		renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_PATCHES, 1);
	}
};

// static
void GasGiant::UpdateAllGasGiants()
{
	PROFILE_SCOPED()
	for (std::vector<GasGiant *>::iterator i = s_allGasGiants.begin(); i != s_allGasGiants.end(); ++i) {
		(*i)->Update();
	}
}

// static
void GasGiant::OnChangeDetailLevel()
{
	s_patchContext.Reset(new GasPatchContext(127));

	// reinit the geosphere terrain data
	for (std::vector<GasGiant *>::iterator i = s_allGasGiants.begin(); i != s_allGasGiants.end(); ++i) {
		// clearout anything we don't need
		(*i)->Reset();

		// reinit the terrain with the new settings
		(*i)->m_terrain.Reset(Terrain::InstanceTerrain((*i)->GetSystemBody()));
	}
}

GasGiant::GasGiant(const SystemBody *body) :
	BaseSphere(body),
	m_hasTempCampos(false),
	m_tempCampos(0.0),
	m_hasGpuJobRequest(false),
	m_timeDelay(s_initialCPUDelayTime)
{
	s_allGasGiants.push_back(this);

	for (int i = 0; i < NUM_PATCHES; i++) {
		m_hasJobRequest[i] = false;
	}

	Random rng(GetSystemBody()->GetSeed() + 4609837);

	const bool bEnableGPUJobs = (Pi::config->Int("EnableGPUJobs") == 1);
	if (bEnableGPUJobs)
		m_timeDelay = s_initialGPUDelayTime + (rng.Double() * (s_initialGPUDelayTime * 0.5));

	//SetUpMaterials is not called until first Render since light count is zero :)

	//BuildFirstPatches and GenerateTexture are only called when we first attempt to render
}

GasGiant::~GasGiant()
{
	// update thread should not be able to access us now, so we can safely continue to delete
	assert(std::count(s_allGasGiants.begin(), s_allGasGiants.end(), this) == 1);
	s_allGasGiants.erase(std::find(s_allGasGiants.begin(), s_allGasGiants.end(), this));
}

void GasGiant::Reset()
{
	{
		for (int i = 0; i < NUM_PATCHES; i++) {
			if (m_hasJobRequest[i] && m_job[i].HasJob())
				m_job[i].GetJob()->OnCancel();
			m_hasJobRequest[i] = false;
		}
	}

	for (int p = 0; p < NUM_PATCHES; p++) {
		// delete patches
		if (m_patches[p]) {
			m_patches[p].reset();
		}
	}

	m_surfaceTextureSmall.Reset();
	m_surfaceTexture.Reset();
	m_surfaceMaterial.Reset();
}

//static
bool GasGiant::OnAddTextureFaceResult(const SystemPath &path, GasGiantJobs::STextureFaceResult *res)
{
	// Find the correct GeoSphere via it's system path, and give it the split result
	for (std::vector<GasGiant *>::iterator i = s_allGasGiants.begin(), iEnd = s_allGasGiants.end(); i != iEnd; ++i) {
		if (path == (*i)->GetSystemBody()->GetPath()) {
			(*i)->AddTextureFaceResult(res);
			return true;
		}
	}
	// GasGiant not found to return the data to, cancel (which deletes it) instead
	if (res) {
		res->OnCancel();
		delete res;
	}
	return false;
}

//static
bool GasGiant::OnAddGPUGenResult(const SystemPath &path, GasGiantJobs::SGPUGenResult *res)
{
	// Find the correct GeoSphere via it's system path, and give it the split result
	for (std::vector<GasGiant *>::iterator i = s_allGasGiants.begin(), iEnd = s_allGasGiants.end(); i != iEnd; ++i) {
		if (path == (*i)->GetSystemBody()->GetPath()) {
			(*i)->AddGPUGenResult(res);
			return true;
		}
	}
	// GasGiant not found to return the data to, cancel (which deletes it) instead
	if (res) {
		res->OnCancel();
		delete res;
	}
	return false;
}

bool GasGiant::AddTextureFaceResult(GasGiantJobs::STextureFaceResult *res)
{
	bool result = false;
	assert(res);
	assert(res->face() >= 0 && res->face() < NUM_PATCHES);
	m_jobColorBuffers[res->face()].reset(res->data().colors);
	m_hasJobRequest[res->face()] = false;
	const Sint32 uvDims = res->data().uvDims;
	assert(uvDims > 0 && uvDims <= 4096);

	// tidyup
	delete res;

	bool bCreateTexture = true;
	for (int i = 0; i < NUM_PATCHES; i++) {
		bCreateTexture = bCreateTexture & (!m_hasJobRequest[i]);
	}

	if (bCreateTexture) {
		// create texture
		const vector2f texSize(1.0f, 1.0f);
		const vector3f dataSize(uvDims, uvDims, 0.0f);
		const Graphics::TextureDescriptor texDesc(
			Graphics::TEXTURE_RGBA_8888,
			dataSize, texSize, Graphics::LINEAR_CLAMP,
			true, false, false, 0, Graphics::TEXTURE_CUBE_MAP);
		m_surfaceTexture.Reset(Pi::renderer->CreateTexture(texDesc));

		// update with buffer from above
		Graphics::TextureCubeData tcd;
		tcd.posX = m_jobColorBuffers[0].get();
		tcd.negX = m_jobColorBuffers[1].get();
		tcd.posY = m_jobColorBuffers[2].get();
		tcd.negY = m_jobColorBuffers[3].get();
		tcd.posZ = m_jobColorBuffers[4].get();
		tcd.negZ = m_jobColorBuffers[5].get();
		m_surfaceTexture->Update(tcd, dataSize, Graphics::TEXTURE_RGBA_8888);

		// cleanup the temporary color buffer storage
		for (int i = 0; i < NUM_PATCHES; i++) {
			m_jobColorBuffers[i].reset();
		}

		// change the planet texture for the new higher resolution texture
		if (m_surfaceMaterial.Get()) {
			m_surfaceMaterial->SetTexture("texture0"_hash,
				m_surfaceTexture.Get());
			m_surfaceTextureSmall.Reset();
		}
	}

	return result;
}

bool GasGiant::AddGPUGenResult(GasGiantJobs::SGPUGenResult *res)
{
	bool result = false;
	assert(res);
	m_hasGpuJobRequest = false;
	assert(!m_gpuJob.HasJob());
#ifndef NDEBUG
	const Sint32 uvDims = res->data().uvDims;
	assert(uvDims > 0 && uvDims <= 4096);
#endif

	// tidyup
	delete res;

	if (m_builtTexture.Valid()) {
		m_surfaceTexture = m_builtTexture;
		m_builtTexture.Reset();

		// these won't be automatically generated otherwise since we used it as a render target
		m_surfaceTexture->BuildMipmaps();

		// change the planet texture for the new higher resolution texture
		if (m_surfaceMaterial.Get()) {
			m_surfaceMaterial->SetTexture("texture0"_hash,
				m_surfaceTexture.Get());
			m_surfaceTextureSmall.Reset();
		}
	}

	return result;
}

// in patch surface coords, [0,1]
inline vector3d GetSpherePointFromCorners(const double x, const double y, const vector3d *corners)
{
	return (corners[0] + x * (1.0 - y) * (corners[1] - corners[0]) + x * y * (corners[2] - corners[0]) + (1.0 - x) * y * (corners[3] - corners[0])).Normalized();
}

void GasGiant::GenerateTexture()
{
	using namespace GasGiantJobs;
	for (int i = 0; i < NUM_PATCHES; i++) {
		if (m_hasGpuJobRequest || m_hasJobRequest[i])
			return;
	}

	const bool bEnableGPUJobs = (Pi::config->Int("EnableGPUJobs") == 1);

	// scope the small texture generation
	{
		const vector2f texSize(1.0f, 1.0f);
		const vector3f dataSize(s_texture_size_small, s_texture_size_small, 0.0f);
		const Graphics::TextureDescriptor texDesc(
			Graphics::TEXTURE_RGBA_8888,
			dataSize, texSize, Graphics::LINEAR_CLAMP,
			false, false, false, 0, Graphics::TEXTURE_CUBE_MAP);
		m_surfaceTextureSmall.Reset(Pi::renderer->CreateTexture(texDesc));

		const Terrain *pTerrain = GetTerrain();
		const double fracStep = 1.0 / double(s_texture_size_small - 1);

		Graphics::TextureCubeData tcd;
		std::unique_ptr<Color[]> bufs[NUM_PATCHES];
		for (int i = 0; i < NUM_PATCHES; i++) {
			Color *colors = new Color[(s_texture_size_small * s_texture_size_small)];
			for (Uint32 v = 0; v < s_texture_size_small; v++) {
				for (Uint32 u = 0; u < s_texture_size_small; u++) {
					// where in this row & column are we now.
					const double ustep = double(u) * fracStep;
					const double vstep = double(v) * fracStep;

					// get point on the surface of the sphere
					const vector3d p = GetSpherePointFromCorners(ustep, vstep, &GetPatchFaces(i, 0));
					// get colour using `p`
					const vector3d colour = pTerrain->GetColor(p, 0.0, p);

					// convert to ubyte and store
					Color *col = colors + (u + (v * s_texture_size_small));
					col[0].r = Uint8(colour.x * 255.0);
					col[0].g = Uint8(colour.y * 255.0);
					col[0].b = Uint8(colour.z * 255.0);
					col[0].a = 255;
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
		m_surfaceTextureSmall->Update(tcd, dataSize, Graphics::TEXTURE_RGBA_8888);
	}

	// create small texture
	if (!bEnableGPUJobs) {
		for (int i = 0; i < NUM_PATCHES; i++) {
			assert(!m_hasJobRequest[i]);
			assert(!m_job[i].HasJob());
			m_hasJobRequest[i] = true;
			GasGiantJobs::STextureFaceRequest *ssrd = new GasGiantJobs::STextureFaceRequest(&GetPatchFaces(i, 0), GetSystemBody()->GetPath(), i, s_texture_size_cpu[Pi::detail.planets], GetTerrain());
			m_job[i] = Pi::GetAsyncJobQueue()->Queue(new GasGiantJobs::SingleTextureFaceJob(ssrd));
		}
	} else {
		// use m_surfaceTexture texture?
		// create texture
		const vector2f texSize(1.0f, 1.0f);
		const vector3f dataSize(s_texture_size_gpu[Pi::detail.planets], s_texture_size_gpu[Pi::detail.planets], 0.0f);
		const Graphics::TextureDescriptor texDesc(
			Graphics::TEXTURE_RGBA_8888,
			dataSize, texSize, Graphics::LINEAR_CLAMP,
			true, false, false, 0, Graphics::TEXTURE_CUBE_MAP);
		m_builtTexture.Reset(Pi::renderer->CreateTexture(texDesc));

		const std::string ColorFracName = GetTerrain()->GetColorFractalName();
		Output("Color Fractal name: %s\n", ColorFracName.c_str());

		Uint32 GasGiantType = GasGiantTexture::GEN_JUPITER_TEXTURE;
		if (ColorFracName == GGSaturn) {
			GasGiantType = GasGiantTexture::GEN_SATURN_TEXTURE;
		} else if (ColorFracName == GGSaturn2) {
			GasGiantType = GasGiantTexture::GEN_SATURN2_TEXTURE;
		} else if (ColorFracName == GGNeptune) {
			GasGiantType = GasGiantTexture::GEN_NEPTUNE_TEXTURE;
		} else if (ColorFracName == GGNeptune2) {
			GasGiantType = GasGiantTexture::GEN_NEPTUNE2_TEXTURE;
		} else if (ColorFracName == GGUranus) {
			GasGiantType = GasGiantTexture::GEN_URANUS_TEXTURE;
		}
		const Uint32 octaves = (Pi::config->Int("AMD_MESA_HACKS") == 0) ? s_noiseOctaves[Pi::detail.planets] : std::min(5U, s_noiseOctaves[Pi::detail.planets]);
		GasGiantType = (octaves << 16) | GasGiantType;

		assert(!m_hasGpuJobRequest);
		assert(!m_gpuJob.HasJob());

		Random rng(GetSystemBody()->GetSeed() + 4609837);
		const std::string parentname = GetSystemBody()->GetParent()->GetName();
		const float hueShift = (parentname == "Sol") ? 0.0f : float(((rng.Double() * 2.0) - 1.0) * 0.9);

		GasGiantJobs::GenFaceQuad *pQuad = new GasGiantJobs::GenFaceQuad(Pi::renderer, vector2f(s_texture_size_gpu[Pi::detail.planets], s_texture_size_gpu[Pi::detail.planets]), GasGiantType);

		GasGiantJobs::SGPUGenRequest *pGPUReq = new GasGiantJobs::SGPUGenRequest(GetSystemBody()->GetPath(), s_texture_size_gpu[Pi::detail.planets], GetTerrain(), GetSystemBody()->GetRadius(), hueShift, pQuad, m_builtTexture.Get());
		m_gpuJob = Pi::GetSyncJobQueue()->Queue(new GasGiantJobs::SingleGPUGenJob(pGPUReq));
		m_hasGpuJobRequest = true;
	}
}

void GasGiant::Update()
{
	PROFILE_SCOPED()
	// assuming that we haven't already generated the texture from the render call.
	if (m_timeDelay > 0.0f) {
		m_timeDelay -= Pi::game->GetTimeStep();
		if (m_timeDelay <= 0.0001f && !m_surfaceTexture.Valid()) {
			// Use the fact that we have a patch as a latch to prevent repeat generation requests.
			if (m_patches[0].get())
				return;

			BuildFirstPatches();
			return;
		}
	}
}

void GasGiant::Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const std::vector<Camera::Shadow> &shadows)
{
	PROFILE_SCOPED()
	if (!m_surfaceTexture.Valid()) {
		// Use the fact that we have a patch as a latch to prevent repeat generation requests.
		if (!m_patches[0].get()) {
			BuildFirstPatches();
		}
	}

	// store this for later usage in the update method.
	m_tempCampos = campos;
	m_hasTempCampos = true;

	matrix4x4d trans = modelView;
	trans.Translate(-campos.x, -campos.y, -campos.z);
	renderer->SetTransform(matrix4x4f(trans)); //need to set this for the following line to work
	matrix4x4d modv = matrix4x4d(renderer->GetTransform());
	matrix4x4d proj = matrix4x4d(renderer->GetProjection());
	Graphics::Frustum frustum(modv, proj);

	// no frustum test of entire gasSphere, since Space::Render does this
	// for each body using its GetBoundingRadius() value

	//First draw - create materials (they do not change afterwards)
	if (!m_surfaceMaterial)
		SetUpMaterials();

	//Update material parameters
	//XXX no need to calculate AP every frame
	auto ap = GetSystemBody()->CalcAtmosphereParams();
	SetMaterialParameters(trans, radius, shadows, ap);
	if (ap.atmosDensity > 0.0) {
		// make atmosphere sphere slightly bigger than required so
		// that the edges of the pixel shader atmosphere jizz doesn't
		// show ugly polygonal angles
		DrawAtmosphereSurface(renderer, trans, campos,
			ap.atmosRadius * 1.01,
			m_atmosphereMaterial);
	}

	Color ambient;

	// save old global ambient
	const Color oldAmbient = renderer->GetAmbientColor();

	{
		// give planet some ambient lighting if the viewer is close to it
		double camdist = campos.Length();
		camdist = 0.1 / (camdist * camdist);
		// why the fuck is this returning 0.1 when we are sat on the planet??
		// JJ: Because campos is relative to a unit-radius planet - 1.0 at the surface
		// XXX oh well, it is the value we want anyway...
		ambient.r = ambient.g = ambient.b = camdist * 255;
		ambient.a = 255;
	}

	renderer->SetAmbientColor(ambient);

	renderer->SetTransform(matrix4x4f(modelView));

	for (int i = 0; i < NUM_PATCHES; i++) {
		m_patches[i]->Render(renderer, campos, modelView, frustum);
	}

	renderer->SetAmbientColor(oldAmbient);

	renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_GASGIANTS, 1);
}

void GasGiant::SetUpMaterials()
{

	// Request material for this planet, with atmosphere.
	// Separate materials for surface and sky.
	Graphics::MaterialDescriptor surfDesc;
	surfDesc.lighting = true;
	surfDesc.quality = Graphics::HAS_ATMOSPHERE | Graphics::HAS_ECLIPSES;
	surfDesc.textures = 1;

	//planetoid with atmosphere
	const AtmosphereParameters ap(GetSystemBody()->CalcAtmosphereParams());
	assert(ap.atmosDensity > 0.0);
	assert(m_surfaceTextureSmall.Valid() || m_surfaceTexture.Valid());

	// surface material is solid
	Graphics::RenderStateDesc rsd;
	m_surfaceMaterial.Reset(Pi::renderer->CreateMaterial("gassphere_base", surfDesc, rsd));
	m_surfaceMaterial->SetTexture("texture0"_hash,
		m_surfaceTexture.Valid() ? m_surfaceTexture.Get() : m_surfaceTextureSmall.Get());

	{
		Graphics::MaterialDescriptor skyDesc;
		skyDesc.lighting = true;
		skyDesc.quality = Graphics::HAS_ECLIPSES;

		// atmosphere is blended
		Graphics::RenderStateDesc rsd;
		rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
		rsd.cullMode = Graphics::CULL_NONE;
		rsd.depthWrite = false;

		const int scattering = Pi::config->Int("RealisticScattering");
		switch (scattering) {
		case 1:
			m_atmosphereMaterial.Reset(Pi::renderer->CreateMaterial("rayleigh_fast", skyDesc, rsd));
			break;
		case 2:
			m_atmosphereMaterial.Reset(Pi::renderer->CreateMaterial("rayleigh_accurate", skyDesc, rsd));
			break;
		default:
			m_atmosphereMaterial.Reset(Pi::renderer->CreateMaterial("geosphere_sky", skyDesc, rsd));
			break;
		}
	}
}

void GasGiant::BuildFirstPatches()
{
	PROFILE_SCOPED()
	if (s_patchContext.Get() == nullptr) {
		s_patchContext.Reset(new GasPatchContext(127));
	}

	using namespace GasGiantJobs;
	m_patches[0].reset(new GasPatch(s_patchContext, this, p1, p2, p3, p4));
	m_patches[1].reset(new GasPatch(s_patchContext, this, p4, p3, p7, p8));
	m_patches[2].reset(new GasPatch(s_patchContext, this, p1, p4, p8, p5));
	m_patches[3].reset(new GasPatch(s_patchContext, this, p2, p1, p5, p6));
	m_patches[4].reset(new GasPatch(s_patchContext, this, p3, p2, p6, p7));
	m_patches[5].reset(new GasPatch(s_patchContext, this, p8, p7, p6, p5));

	GenerateTexture();
}

void GasGiant::Init()
{
	IniConfig cfg;
	cfg.Read(FileSystem::gameDataFiles, "configs/GasGiants.ini");
	// NB: limit the ranges of all values loaded from the file
	// NB: round to the nearest power of 2 for all texture sizes
	s_texture_size_small = ceil_pow2(Clamp(cfg.Int("texture_size_small", 16), 16, 64));

	SplitData(cfg.String("texture_size_0"), s_texture_size_cpu[0], s_texture_size_gpu[0], s_noiseOctaves[0]);
	SplitData(cfg.String("texture_size_1"), s_texture_size_cpu[1], s_texture_size_gpu[1], s_noiseOctaves[1]);
	SplitData(cfg.String("texture_size_2"), s_texture_size_cpu[2], s_texture_size_gpu[2], s_noiseOctaves[2]);
	SplitData(cfg.String("texture_size_3"), s_texture_size_cpu[3], s_texture_size_gpu[3], s_noiseOctaves[3]);
	SplitData(cfg.String("texture_size_4"), s_texture_size_cpu[4], s_texture_size_gpu[4], s_noiseOctaves[4]);

	s_initialCPUDelayTime = Clamp(cfg.Float("cpu_delay_time", 60.0f), 0.0f, 120.0f);
	s_initialGPUDelayTime = Clamp(cfg.Float("gpu_delay_time", 5.0f), 0.0f, 120.0f);

	if (s_patchContext.Get() == nullptr) {
		s_patchContext.Reset(new GasPatchContext(127));
	}
	CreateRenderTarget(s_texture_size_gpu[Pi::detail.planets], s_texture_size_gpu[Pi::detail.planets]);
}

void GasGiant::Uninit()
{
	s_patchContext.Reset();
}

//static
void GasGiant::CreateRenderTarget(const Uint16 width, const Uint16 height)
{
	/*	@fluffyfreak here's a rendertarget implementation you can use for oculusing and other things. It's pretty simple:
		 - fill out a RenderTargetDesc struct and call Renderer::CreateRenderTarget
		 - pass target to Renderer::SetRenderTarget to start rendering to texture
		 - set up viewport, clear etc, then draw as usual
		 - SetRenderTarget(0) to resume render to screen
		 - you can access the attached texture with GetColorTexture to use it with a material
		You can reuse the same target with multiple textures.
		In that case, leave the color format to NONE so the initial texture is not created, then use SetColorTexture to attach your own.
	*/

	// Complete the RT description so we can request a buffer.
	// NB: we don't want it to create use a texture because we share it with the textured quad created above.
	Graphics::RenderTargetDesc rtDesc(
		width,
		height,
		Graphics::TEXTURE_NONE, // don't create a texture
		Graphics::TEXTURE_NONE, // don't create a depth buffer
		false);
	s_renderTarget = Pi::renderer->CreateRenderTarget(rtDesc);
}

//static
void GasGiant::SetRenderTargetCubemap(const Uint32 face, Graphics::Texture *pTexture, const bool unBind /*= true*/)
{
	s_renderTarget->SetCubeFaceTexture(face, pTexture);
}

// static
Graphics::RenderTarget *GasGiant::GetRenderTarget()
{
	return s_renderTarget;
}
