// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GeoSphere.h"

#include "GameConfig.h"
#include "GeoPatch.h"
#include "GeoPatchContext.h"
#include "GeoPatchJobs.h"
#include "Pi.h"
#include "RefCounted.h"
#include "galaxy/AtmosphereParameters.h"
#include "galaxy/StarSystem.h"
#include "graphics/Drawables.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "JsonUtils.h"
#include "perlin.h"
#include "StringF.h"
#include "utils.h"
#include "vcacheopt/vcacheopt.h"
#include <algorithm>
#include <deque>

RefCountedPtr<GeoPatchContext> GeoSphere::s_patchContext;

namespace {
	// points around a unit sphere for sampling height data at uniformally
	double g_sampleHeights[41] = {};
	const vector3d g_samplePoints[] = {
		{ vector3d(-0.160622, -0.160622, -0.160622) },
		{ vector3d(-0.16246, -0.16246, -0.16246) },
		{ vector3d(-0.259892, -0.259892, -0.259892) },
		{ vector3d(-0.262866, -0.262866, -0.262866) },
		{ vector3d(-0.273266, -0.273266, -0.273266) },
		{ vector3d(-0.309017, -0.309017, -0.309017) },
		{ vector3d(-0.425325, -0.425325, -0.425325) },
		{ vector3d(-0.433889, -0.433889, -0.433889) },
		{ vector3d(-0.525731, -0.525731, -0.525731) },
		{ vector3d(-0.587785, -0.587785, -0.587785) },
		{ vector3d(-0.5, -0.5, -0.5) },
		{ vector3d(-0.688191, -0.688191, -0.688191) },
		{ vector3d(-0.69378, -0.69378, -0.69378) },
		{ vector3d(-0.702046, -0.702046, -0.702046) },
		{ vector3d(-0.809017, -0.809017, -0.809017) },
		{ vector3d(-0.850651, -0.850651, -0.850651) },
		{ vector3d(-0.862669, -0.862669, -0.862669) },
		{ vector3d(-0.951057, -0.951057, -0.951057) },
		{ vector3d(-0.961938, -0.961938, -0.961938) },
		{ vector3d(-1., -1., -1.) },
		{ vector3d(0.160622, 0.160622, 0.160622) },
		{ vector3d(0.16246, 0.16246, 0.16246) },
		{ vector3d(0.259892, 0.259892, 0.259892) },
		{ vector3d(0.262866, 0.262866, 0.262866) },
		{ vector3d(0.273266, 0.273266, 0.273266) },
		{ vector3d(0.309017, 0.309017, 0.309017) },
		{ vector3d(0.425325, 0.425325, 0.425325) },
		{ vector3d(0.433889, 0.433889, 0.433889) },
		{ vector3d(0.525731, 0.525731, 0.525731) },
		{ vector3d(0.587785, 0.587785, 0.587785) },
		{ vector3d(0.5, 0.5, 0.5) },
		{ vector3d(0.688191, 0.688191, 0.688191) },
		{ vector3d(0.69378, 0.69378, 0.69378) },
		{ vector3d(0.702046, 0.702046, 0.702046) },
		{ vector3d(0.809017, 0.809017, 0.809017) },
		{ vector3d(0.850651, 0.850651, 0.850651) },
		{ vector3d(0.862669, 0.862669, 0.862669) },
		{ vector3d(0.951057, 0.951057, 0.951057) },
		{ vector3d(0.961938, 0.961938, 0.961938) },
		{ vector3d(0., 0., 0.) },
		{ vector3d(1., 1., 1.) },
	};
} //namespace

// must be odd numbers
static const int detail_edgeLen[5] = {
	//7, 15, 25, 35, 55 -- old non power-of-2+1 values
	// some detail settings duplicated intentionally
	// in real terms provides only 3 settings
	// however this value is still used for gas giants
	// with 5 distinct settings elsewhere
	9, 17, 17, 33, 33
};

static const double gs_targetPatchTriLength(100.0);
static std::vector<GeoSphere *> s_allGeospheres;
static Uint32 s_debugFlags = GeoSphere::DebugFlags::DEBUG_NONE;

void GeoSphere::InitGeoSphere()
{
	s_patchContext.Reset(new GeoPatchContext(detail_edgeLen[Pi::detail.planets > 4 ? 4 : Pi::detail.planets]));
}

void GeoSphere::UninitGeoSphere()
{
	assert(s_patchContext.Unique());
	s_patchContext.Reset();
}

static void print_info(const SystemBody *sbody, const Terrain *terrain)
{
	Log::Verbose(
		"Geosphere Init for {}:\n" \
		"\theight fractal: {}\n" \
		"\tcolour fractal: {}\n" \
		"\tseed: {}\n",
		sbody->GetName(), terrain->GetHeightFractalName(), terrain->GetColorFractalName(), sbody->GetSeed());
}

// static
void GeoSphere::UpdateAllGeoSpheres()
{
	PROFILE_SCOPED()
	for (std::vector<GeoSphere *>::iterator i = s_allGeospheres.begin(); i != s_allGeospheres.end(); ++i) {
		(*i)->Update();
	}
}

// static
void GeoSphere::OnChangeGeoSphereDetailLevel()
{
	s_patchContext.Reset(new GeoPatchContext(detail_edgeLen[Pi::detail.planets > 4 ? 4 : Pi::detail.planets]));

	// reinit the geosphere terrain data
	for (std::vector<GeoSphere *>::iterator i = s_allGeospheres.begin(); i != s_allGeospheres.end(); ++i) {
		// clearout anything we don't need
		(*i)->Reset();

		// reinit the terrain with the new settings
		(*i)->m_terrain.Reset(Terrain::InstanceTerrain((*i)->GetSystemBody()));
		print_info((*i)->GetSystemBody(), (*i)->m_terrain.Get());

		// Reload the atmosphere material (scattering option)
		if ((*i)->m_atmosphereMaterial.Valid()) {
			(*i)->CreateAtmosphereMaterial();
		}
	}
}

//static
bool GeoSphere::OnAddQuadSplitResult(const SystemPath &path, SQuadSplitResult *res)
{
	// Find the correct GeoSphere via it's system path, and give it the split result
	for (std::vector<GeoSphere *>::iterator i = s_allGeospheres.begin(), iEnd = s_allGeospheres.end(); i != iEnd; ++i) {
		if (path == (*i)->GetSystemBody()->GetPath()) {
			(*i)->AddQuadSplitResult(res);
			return true;
		}
	}
	// GeoSphere not found to return the data to, cancel and delete it instead
	if (res) {
		res->OnCancel();
		delete res;
	}
	return false;
}

//static
bool GeoSphere::OnAddSingleSplitResult(const SystemPath &path, SSingleSplitResult *res)
{
	// Find the correct GeoSphere via it's system path, and give it the split result
	for (std::vector<GeoSphere *>::iterator i = s_allGeospheres.begin(), iEnd = s_allGeospheres.end(); i != iEnd; ++i) {
		if (path == (*i)->GetSystemBody()->GetPath()) {
			(*i)->AddSingleSplitResult(res);
			return true;
		}
	}
	// GeoSphere not found to return the data to, cancel and delete it instead
	if (res) {
		res->OnCancel();
		delete res;
	}
	return false;
}

//static
void GeoSphere::SetDebugFlags(Uint32 flags)
{
	s_debugFlags = flags;
}

//static
Uint32 GeoSphere::GetDebugFlags()
{
	return s_debugFlags;
}

void GeoSphere::Reset()
{
	{
		std::deque<SSingleSplitResult *>::iterator iter = mSingleSplitResults.begin();
		while (iter != mSingleSplitResults.end()) {
			// finally pass SplitResults
			SSingleSplitResult *psr = (*iter);
			assert(psr);

			psr->OnCancel();

			// tidyup
			delete psr;

			// Next!
			++iter;
		}
		mSingleSplitResults.clear();
	}

	{
		std::deque<SQuadSplitResult *>::iterator iter = mQuadSplitResults.begin();
		while (iter != mQuadSplitResults.end()) {
			// finally pass SplitResults
			SQuadSplitResult *psr = (*iter);
			assert(psr);

			psr->OnCancel();

			// tidyup
			delete psr;

			// Next!
			++iter;
		}
		mQuadSplitResults.clear();
	}

	for (int p = 0; p < NUM_PATCHES; p++) {
		// delete patches
		if (m_patches[p]) {
			m_patches[p].reset();
		}
	}

	CalculateMaxPatchDepth();

	m_visiblePatches.reserve(1024);

	m_initStage = eBuildFirstPatches;
}

GeoSphere::GeoSphere(const SystemBody *body) :
	BaseSphere(body),
	m_hasTempCampos(false),
	m_tempCampos(0.0),
	m_tempFrustum(800, 600, 0.5, 1.0, 1000.0),
	m_initStage(eBuildFirstPatches),
	m_maxDepth(0)
{
	print_info(body, m_terrain.Get());

	s_allGeospheres.emplace_back(this);

	CalculateMaxPatchDepth();

	m_visiblePatches.reserve(1024);

	if (Pi::config->Int("SortGeoPatches") == 0) {
		SetDebugFlags(GetDebugFlags() & ~DebugFlags::DEBUG_SORTGEOPATCHES);
	} else {
		SetDebugFlags(GetDebugFlags() | DebugFlags::DEBUG_SORTGEOPATCHES);
	}

	//SetUpMaterials is not called until first Render since light count is zero :)
}

GeoSphere::~GeoSphere()
{
	// update thread should not be able to access us now, so we can safely continue to delete
	assert(std::count(s_allGeospheres.begin(), s_allGeospheres.end(), this) == 1);
	s_allGeospheres.erase(std::find(s_allGeospheres.begin(), s_allGeospheres.end(), this));
}

bool GeoSphere::AddQuadSplitResult(SQuadSplitResult *res)
{
	bool result = false;
	assert(res);
	assert(mQuadSplitResults.size() < MAX_SPLIT_OPERATIONS);
	if (mQuadSplitResults.size() < MAX_SPLIT_OPERATIONS) {
		mQuadSplitResults.emplace_back(res);
		result = true;
	}
	return result;
}

bool GeoSphere::AddSingleSplitResult(SSingleSplitResult *res)
{
	bool result = false;
	assert(res);
	assert(mSingleSplitResults.size() < MAX_SPLIT_OPERATIONS);
	if (mSingleSplitResults.size() < MAX_SPLIT_OPERATIONS) {
		mSingleSplitResults.emplace_back(res);
		result = true;
	}
	return result;
}

void GeoSphere::ProcessSplitResults()
{
	PROFILE_SCOPED()
	// now handle the single split results that define the base level of the quad tree
	{
		std::deque<SSingleSplitResult *>::iterator iter = mSingleSplitResults.begin();
		while (iter != mSingleSplitResults.end()) {
			// finally pass SplitResults
			SSingleSplitResult *psr = (*iter);
			assert(psr);

			const int32_t faceIdx = psr->face();
			if (m_patches[faceIdx]) {
				m_patches[faceIdx]->ReceiveHeightmap(psr);
			} else {
				psr->OnCancel();
			}

			// tidyup
			delete psr;

			// Next!
			++iter;
		}
		mSingleSplitResults.clear();
	}

	// now handle the quad split results
	{
		std::deque<SQuadSplitResult *>::iterator iter = mQuadSplitResults.begin();
		while (iter != mQuadSplitResults.end()) {
			// finally pass SplitResults
			SQuadSplitResult *psr = (*iter);
			assert(psr);

			const int32_t faceIdx = psr->face();
			if (m_patches[faceIdx]) {
				m_patches[faceIdx]->ReceiveHeightmaps(psr);
			} else {
				psr->OnCancel();
			}

			// tidyup
			delete psr;

			// Next!
			++iter;
		}
		mQuadSplitResults.clear();
	}
}

void GeoSphere::BuildFirstPatches()
{
	PROFILE_SCOPED()
	assert(!m_patches[0]);
	if (m_patches[0])
		return;

	CalculateMaxPatchDepth();

	// generate root face patches of the cube/sphere
	static const vector3d p1 = (vector3d(1, 1, 1)).Normalized();
	static const vector3d p2 = (vector3d(-1, 1, 1)).Normalized();
	static const vector3d p3 = (vector3d(-1, -1, 1)).Normalized();
	static const vector3d p4 = (vector3d(1, -1, 1)).Normalized();
	static const vector3d p5 = (vector3d(1, 1, -1)).Normalized();
	static const vector3d p6 = (vector3d(-1, 1, -1)).Normalized();
	static const vector3d p7 = (vector3d(-1, -1, -1)).Normalized();
	static const vector3d p8 = (vector3d(1, -1, -1)).Normalized();

	const uint64_t maxShiftDepth = GeoPatchID::MAX_SHIFT_DEPTH;

	m_patches[0].reset(new GeoPatch(s_patchContext, this, p1, p2, p3, p4, 0, (0ULL << maxShiftDepth)));
	m_patches[1].reset(new GeoPatch(s_patchContext, this, p4, p3, p7, p8, 0, (1ULL << maxShiftDepth)));
	m_patches[2].reset(new GeoPatch(s_patchContext, this, p1, p4, p8, p5, 0, (2ULL << maxShiftDepth)));
	m_patches[3].reset(new GeoPatch(s_patchContext, this, p2, p1, p5, p6, 0, (3ULL << maxShiftDepth)));
	m_patches[4].reset(new GeoPatch(s_patchContext, this, p3, p2, p6, p7, 0, (4ULL << maxShiftDepth)));
	m_patches[5].reset(new GeoPatch(s_patchContext, this, p8, p7, p6, p5, 0, (5ULL << maxShiftDepth)));

	for (int i = 0; i < NUM_PATCHES; i++) {
		m_patches[i]->RequestSinglePatch();
	}

	m_initStage = eRequestedFirstPatches;
}

void GeoSphere::CalculateMaxPatchDepth()
{
	const double circumference = 2.0 * M_PI * m_sbody->GetRadius();
	// calculate length of each edge segment (quad) times 4 due to that being the number around the sphere (1 per side, 4 sides for Root).
	double edgeMetres = circumference / double(s_patchContext->GetEdgeLen() * 8);
	// find out what depth we reach the desired resolution
	while (edgeMetres > gs_targetPatchTriLength && m_maxDepth < GEOPATCH_MAX_DEPTH) {
		edgeMetres *= 0.5;
		++m_maxDepth;
	}
}

void GeoSphere::Update()
{
	switch (m_initStage) {
	case eBuildFirstPatches:
		BuildFirstPatches();
		break;
	case eRequestedFirstPatches: {
		ProcessSplitResults();
		uint8_t numValidPatches = 0;
		for (int i = 0; i < NUM_PATCHES; i++) {
			if (m_patches[i]->HasHeightData()) {
				++numValidPatches;
			}
		}
		m_initStage = (NUM_PATCHES == numValidPatches) ? eReceivedFirstPatches : eRequestedFirstPatches;
	} break;
	case eReceivedFirstPatches: {
		for (int i = 0; i < NUM_PATCHES; i++) {
			m_patches[i]->SetNeedToUpdateVBOs();
		}
		m_initStage = eDefaultUpdateState;
	} break;
	case eDefaultUpdateState:
		if (m_hasTempCampos) {
			ProcessSplitResults();
			for (int i = 0; i < NUM_PATCHES; i++) {
				m_patches[i]->LODUpdate(m_tempCampos, m_tempFrustum);
			}
			ProcessQuadSplitRequests();
		}
		break;
	}
}

void GeoSphere::AddQuadSplitRequest(double dist, SQuadSplitRequest *pReq, GeoPatch *pPatch)
{
	mQuadSplitRequests.emplace_back(dist, pReq, pPatch);
}

void GeoSphere::ProcessQuadSplitRequests()
{
	std::sort(mQuadSplitRequests.begin(), mQuadSplitRequests.end(), [](const TDistanceRequest &a, const TDistanceRequest &b) { return a.mDistance < b.mDistance; });

	for (auto iter : mQuadSplitRequests) {
		SQuadSplitRequest *ssrd = iter.mpRequest;
		iter.mpRequester->ReceiveJobHandle(Pi::GetAsyncJobQueue()->Queue(new QuadPatchJob(ssrd)));
	}
	mQuadSplitRequests.clear();
}

void GeoSphere::Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const std::vector<Camera::Shadow> &shadows)
{
	PROFILE_SCOPED()
	// store this for later usage in the update method.
	m_tempCampos = campos;
	m_hasTempCampos = true;

	if (m_initStage < eDefaultUpdateState)
		return;

	matrix4x4d trans = modelView;
	trans.Translate(-campos.x, -campos.y, -campos.z);
	renderer->SetTransform(matrix4x4f(trans)); //need to set this for the following line to work
	matrix4x4d modv = matrix4x4d(renderer->GetTransform());
	matrix4x4d proj = matrix4x4d(renderer->GetProjection());
	Graphics::Frustum frustum(modv, proj);
	m_tempFrustum = frustum;

	// no frustum test of entire geosphere, since Space::Render does this
	// for each body using its GetBoundingRadius() value

	//First draw - create materials (they do not change afterwards)
	if (!m_surfaceMaterial)
		SetUpMaterials();

	//Update material parameters
	SetMaterialParameters(trans, radius, shadows, m_atmosphereParameters);

	if (m_atmosphereMaterial.Valid() && m_atmosphereParameters.atmosDensity > 0.0) {
		// make atmosphere sphere slightly bigger than required so
		// that the edges of the pixel shader atmosphere jizz doesn't
		// show ugly polygonal angles
		DrawAtmosphereSurface(renderer, trans, campos,
			m_atmosphereParameters.atmosRadius * 1.02,
			m_atmosphereMaterial);
	}

	Color ambient;
	Color &emission = m_surfaceMaterial->emissive;

	// save old global ambient
	const Color oldAmbient = renderer->GetAmbientColor();

	if ((GetSystemBody()->GetSuperType() == SystemBody::SUPERTYPE_STAR) || (GetSystemBody()->GetType() == SystemBody::TYPE_BROWN_DWARF)) {
		// stars should emit light and terrain should be visible from distance
		ambient.r = ambient.g = ambient.b = 51;
		ambient.a = 255;
		emission = GetSystemBody()->GetStarColor();
		emission.a = 255;
	} else {
		// give planet some ambient lighting if the viewer is close to it
		double camdist = 0.1 / campos.LengthSqr();
		// why the fuck is this returning 0.1 when we are sat on the planet??
		// JJ: Because campos is relative to a unit-radius planet - 1.0 at the surface
		// XXX oh well, it is the value we want anyway...
		ambient.r = ambient.g = ambient.b = camdist * 255;
		ambient.a = 255;
	}

	renderer->SetAmbientColor(ambient);

	renderer->SetTransform(matrix4x4f(modelView));

	if (s_debugFlags & GeoSphere::DebugFlags::DEBUG_WIREFRAME)
		renderer->SetWireFrameMode(true);

	if (s_debugFlags & GeoSphere::DebugFlags::DEBUG_SORTGEOPATCHES) {
		// Gather the patches that could be rendered
		for (int i = 0; i < NUM_PATCHES; i++) {
			m_patches[i]->GatherRenderablePatches(m_visiblePatches, renderer, campos, frustum);
		}

		// distance sort the patches
		std::sort(m_visiblePatches.begin(), m_visiblePatches.end(), [&, campos](const std::pair<double, GeoPatch *> &a, const std::pair<double, GeoPatch *> &b) {
			return (a.first) < (b.first);
		});

		// cull occluded patches somehow?
		// create frustum from corner points, something vertical, and the campos??? Cull anything within that frustum?

		// render the sorted patches
		for (std::pair<double, GeoPatch *> &pPatch : m_visiblePatches) {
			pPatch.second->RenderImmediate(renderer, campos, modelView);
		}

		// must clear this after each render otherwise it just accumulates every patch ever drawn!
		m_visiblePatches.clear();
	} else {
		for (int i = 0; i < NUM_PATCHES; i++) {
			m_patches[i]->Render(renderer, campos, modelView, frustum);
		}
	}

	if (s_debugFlags & GeoSphere::DebugFlags::DEBUG_WIREFRAME)
		renderer->SetWireFrameMode(false);

	renderer->SetAmbientColor(oldAmbient);

	renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_PLANETS, 1);
}

void GeoSphere::LoadTerrainJSON(const std::string &path)
{
	Json rootNode = JsonUtils::LoadJsonDataFile(path);

	if (!rootNode.is_object()) {
		Output("couldn't open json terrain definition '%s'\n", path.c_str());
		return;
	}

	if (rootNode.size() == 0) {
		Output("couldn't read json terrain definition '%s'\n", path.c_str());
		return;
	}

	Output("\n%s\n", path.c_str());

	// load the look-up-table
	Json lut = rootNode["LUT"];
	assert(!lut.empty() && lut.is_string());
	m_surfaceLUT.Reset(Graphics::TextureBuilder::LookUpTable(stringf("textures/terrain/%0", lut.get<std::string>())).GetOrCreateTexture(Pi::renderer, "lut"));

	// what detail textures does this terrain use
	const std::string detailTextureHigh = rootNode["detailTextureHigh"];
	m_texHi.Reset(Graphics::TextureBuilder::Model(stringf("textures/%0", detailTextureHigh)).GetOrCreateTexture(Pi::renderer, "model"));
	const std::string detailTextureLow = rootNode["detailTextureLow"];
	m_texLo.Reset(Graphics::TextureBuilder::Model(stringf("textures/%0", detailTextureLow)).GetOrCreateTexture(Pi::renderer, "model"));

	// find the texture filenames this terrain uses
	const int numberLayers = rootNode["Layers"];

	struct Atlas {
		struct TextureCandidate {
			std::string textureName;
			int minAverageTemp;
			int minLife;
			int minVolatileGas;
			int minVolatileIces;
			int minVolatileLiquid;
			int minVolcanicity;
		};

		std::string description;
		std::vector<TextureCandidate> textureCandidates;
	};
	std::vector<Atlas> atlases;
	atlases.reserve(numberLayers);

	Json atlas = rootNode["atlas"];
	if (!atlas.empty()) {
		for (auto j = atlas.begin(), jEnd = atlas.end(); j != jEnd; ++j) {
			Atlas newAtlas;
			Json description = (*j)["description"];
			if (!description.empty()) {
				const std::string desc = description;
				newAtlas.description = desc;
			}

			Json textures = (*j)["textures"];
			if (!textures.empty()) {
				for (auto tex = textures.begin(), texEnd = textures.end(); tex != texEnd; ++tex) {
					// the texture itself
					Atlas::TextureCandidate tc;
					std::string textureName = (*tex)["texture"];
					tc.textureName = textureName;
					tc.minAverageTemp = (*tex)["minAverageTemp"];
					tc.minLife = (*tex)["minLife"];
					tc.minVolatileGas = (*tex)["minVolatileGas"];
					tc.minVolatileIces = (*tex)["minVolatileIces"];
					tc.minVolatileLiquid = (*tex)["minVolatileLiquid"];
					tc.minVolcanicity = (*tex)["minVolcanicity"];

					newAtlas.textureCandidates.push_back(tc);
				}
			}
			atlases.push_back(newAtlas);
		}
	}

	// choose the textures based on the planets properties
	std::vector<std::string> textureFilenames;
	textureFilenames.reserve(numberLayers);
	// do the choosing...
	//???
	for (auto atlas : atlases) {
		//???
		for (auto tc : atlas.textureCandidates) {
			// just take the first one for now
			textureFilenames.push_back(stringf("textures/terrain/%0", tc.textureName));
			break;
		}
	}

	// load 'em
	m_surfaceAtlas.Reset(Graphics::TextureBuilder::Array(textureFilenames, numberLayers).GetOrCreateTexture(Pi::renderer, "array"));
}

void GeoSphere::SetUpMaterials()
{
	// XXX: this has to be synced with the vertex format used in GeoPatch
	auto vtxFormat = Graphics::VertexFormatDesc::FromAttribSet(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0);

	m_atmosphereParameters = GetSystemBody()->CalcAtmosphereParams();
	// normal star has a different setup path than geosphere terrain does
	if (GetSystemBody()->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
		Graphics::MaterialDescriptor surfDesc;
		surfDesc.lighting = false;
		Graphics::RenderStateDesc rsd;
		m_surfaceMaterial.Reset(Pi::renderer->CreateMaterial("geosphere_star", surfDesc, rsd, vtxFormat));
	} else {
		// Request material for this star or planet, with or without
		// atmosphere. Separate material for surface and sky.
		Graphics::MaterialDescriptor surfDesc;
		const Uint32 effect_flags = m_terrain->GetSurfaceEffects();
		if (effect_flags & Terrain::EFFECT_LAVA)
			surfDesc.effect = Graphics::EFFECT_GEOSPHERE_TERRAIN_WITH_LAVA;
		else if (effect_flags & Terrain::EFFECT_WATER)
			surfDesc.effect = Graphics::EFFECT_GEOSPHERE_TERRAIN_WITH_WATER;
		else
			surfDesc.effect = Graphics::EFFECT_GEOSPHERE_TERRAIN;

		if ((GetSystemBody()->GetType() == SystemBody::TYPE_BROWN_DWARF) ||
			(GetSystemBody()->GetType() == SystemBody::TYPE_STAR_M)) {
			//dim star (emits and receives light)
			surfDesc.lighting = true;
			// emits light, so atmosphere should not affect visible light
			surfDesc.quality &= ~Graphics::HAS_ATMOSPHERE;
		} else {
			//planetoid with or without atmosphere
			surfDesc.lighting = true;
			if (m_atmosphereParameters.atmosDensity > 0.0) {
				surfDesc.quality |= Graphics::HAS_ATMOSPHERE;
			}
		}

		//solid blendmode
		Graphics::RenderStateDesc rsd;
		surfDesc.quality |= Graphics::HAS_ECLIPSES;
		m_surfaceMaterial.Reset(Pi::renderer->CreateMaterial("geosphere_terrain", surfDesc, rsd, vtxFormat));

		m_texHi.Reset(Graphics::TextureBuilder::Model("textures/high.dds").GetOrCreateTexture(Pi::renderer, "model"));
		m_texLo.Reset(Graphics::TextureBuilder::Model("textures/low.dds").GetOrCreateTexture(Pi::renderer, "model"));
		//m_surfaceLUT.Reset(Graphics::TextureBuilder::LookUpTable("textures/terrain/terrainLUT.png").GetOrCreateTexture(Pi::renderer, "lut"));
		//const std::vector<std::string> &filenames{ std::string("textures/terrain/atlas.dds") };
		//m_surfaceAtlas.Reset(Graphics::TextureBuilder::Array(filenames, 16).GetOrCreateTexture(Pi::renderer, "array"));
		LoadTerrainJSON(stringf("terrain/%0.json", m_terrain->GetColorFractalName()));

		m_surfaceMaterial->SetTexture("texture0"_hash, m_texHi.Get());
		m_surfaceMaterial->SetTexture("texture1"_hash, m_texLo.Get());
		m_surfaceMaterial->SetTexture("texture2"_hash, m_surfaceLUT.Get());
		m_surfaceMaterial->SetTexture("texture3"_hash, m_surfaceAtlas.Get());
	}

	CreateAtmosphereMaterial();

	// Horrible sampling hackyness that attempts to establish the min and max heights around the planet
	// by sampling the terrain at N points, 41 at time of writing this comment.
	const size_t numSamplePts = sizeof(g_samplePoints) / sizeof(vector3d);
	double minh = DBL_MAX, maxh = DBL_MIN;
	m_terrain->GetHeights(&g_samplePoints[0], &g_sampleHeights[0], numSamplePts);
	for (int sp = 0; sp < numSamplePts; sp++) {
		const double h = g_sampleHeights[sp];
		minh = std::min(minh, h);
		maxh = std::max(maxh, h);
	}
	//Output("min (%.3lf), max (%.3lf), inverse max (%.3lf)\n", minh, maxh, 1.0 / maxh);
	m_heightNormaliserMin = minh;
	m_heightNormaliserMax = 1.0 / maxh;
}

void GeoSphere::CreateAtmosphereMaterial()
{
	Graphics::RenderStateDesc rsd;
	// atmosphere is blended over the background
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	rsd.cullMode = Graphics::CULL_FRONT;
	rsd.depthWrite = false;

	Graphics::MaterialDescriptor skyDesc;
	skyDesc.effect = Graphics::EFFECT_GEOSPHERE_SKY;
	skyDesc.lighting = true;
	skyDesc.quality |= Graphics::HAS_ECLIPSES;

	Graphics::VertexFormatDesc atmosVtxFmt = m_atmos->GetVertexFormat();

	const int scattering = Pi::config->Int("RealisticScattering");
	switch (scattering) {
	case 1:
		m_atmosphereMaterial.Reset(Pi::renderer->CreateMaterial("rayleigh_fast", skyDesc, rsd, atmosVtxFmt));
		break;
	case 2:
		m_atmosphereMaterial.Reset(Pi::renderer->CreateMaterial("rayleigh_accurate", skyDesc, rsd, atmosVtxFmt));
		break;
	default:
		m_atmosphereMaterial.Reset(Pi::renderer->CreateMaterial("geosphere_sky", skyDesc, rsd, atmosVtxFmt));
		break;
	}
}
