// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "GeoSphere.h"
#include "GeoPatchContext.h"
#include "GeoPatch.h"
#include "GeoPatchJobs.h"
#include "CloudJobs.h"
#include "perlin.h"
#include "Pi.h"
#include "RefCounted.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/Texture.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"
#include "vcacheopt/vcacheopt.h"
#include <deque>
#include <algorithm>

#define DUMP_TO_TEXTURE false
#if DUMP_TO_TEXTURE
#include "FileSystem.h"
#include "PngWriter.h"
#include "graphics/opengl/TextureGL.h"
void textureDump(const char* destFile, const int width, const int height, const Color* buf)
{
	const std::string dir = "generated_textures";
	FileSystem::userFiles.MakeDirectory(dir);
	const std::string fname = FileSystem::JoinPathBelow(dir, destFile);

	// pad rows to 4 bytes, which is the default row alignment for OpenGL
	//const int stride = (3*width + 3) & ~3;
	const int stride = width * 4;

	write_png(FileSystem::userFiles, fname, &buf[0].r, width, height, stride, 4);

	printf("texture %s saved\n", fname.c_str());
}
#endif

RefCountedPtr<GeoPatchContext> GeoSphere::s_patchContext;

// must be odd numbers
static const int detail_edgeLen[5] = {
	7, 15, 25, 35, 55
};

static const double gs_targetPatchTriLength(100.0);

#define PRINT_VECTOR(_v) Output("%f,%f,%f\n", (_v).x, (_v).y, (_v).z);

static const int geo_sphere_edge_friends[NUM_PATCHES][4] = {
	{ 3, 4, 1, 2 },
	{ 0, 4, 5, 2 },
	{ 0, 1, 5, 3 },
	{ 0, 2, 5, 4 },
	{ 0, 3, 5, 1 },
	{ 1, 4, 3, 2 }
};

static std::vector<GeoSphere*> s_allGeospheres;

void GeoSphere::Init()
{
	s_patchContext.Reset(new GeoPatchContext(detail_edgeLen[Pi::detail.planets > 4 ? 4 : Pi::detail.planets]));
}

void GeoSphere::Uninit()
{
	assert (s_patchContext.Unique());
	s_patchContext.Reset();
}

static void print_info(const SystemBody *sbody, const Terrain *terrain)
{
	Output(
		"%s:\n"
		"    height fractal: %s\n"
		"    colour fractal: %s\n"
		"    seed: %u\n",
		sbody->GetName().c_str(), terrain->GetHeightFractalName(), terrain->GetColorFractalName(), sbody->GetSeed());
}

// static
void GeoSphere::UpdateAllGeoSpheres()
{
	PROFILE_SCOPED()
	for(std::vector<GeoSphere*>::iterator i = s_allGeospheres.begin(); i != s_allGeospheres.end(); ++i)
	{
		(*i)->Update();
	}
}

// static
void GeoSphere::OnChangeDetailLevel()
{
	s_patchContext.Reset(new GeoPatchContext(detail_edgeLen[Pi::detail.planets > 4 ? 4 : Pi::detail.planets]));

	// reinit the geosphere terrain data
	for(std::vector<GeoSphere*>::iterator i = s_allGeospheres.begin(); i != s_allGeospheres.end(); ++i)
	{
		// clearout anything we don't need
		(*i)->Reset();

		// reinit the terrain with the new settings
		(*i)->m_terrain.Reset(Terrain::InstanceTerrain((*i)->GetSystemBody()));
		print_info((*i)->GetSystemBody(), (*i)->m_terrain.Get());
	}
}

//static
bool GeoSphere::OnAddQuadSplitResult(const SystemPath &path, SQuadSplitResult *res)
{
	// Find the correct GeoSphere via it's system path, and give it the split result
	for(std::vector<GeoSphere*>::iterator i=s_allGeospheres.begin(), iEnd=s_allGeospheres.end(); i!=iEnd; ++i) {
		if( path == (*i)->GetSystemBody()->GetPath() ) {
			(*i)->AddQuadSplitResult(res);
			return true;
		}
	}
	// GeoSphere not found to return the data to, cancel and delete it instead
	if( res ) {
		res->OnCancel();
		delete res;
	}
	return false;
}

//static
bool GeoSphere::OnAddSingleSplitResult(const SystemPath &path, SSingleSplitResult *res)
{
	// Find the correct GeoSphere via it's system path, and give it the split result
	for(std::vector<GeoSphere*>::iterator i=s_allGeospheres.begin(), iEnd=s_allGeospheres.end(); i!=iEnd; ++i) {
		if( path == (*i)->GetSystemBody()->GetPath() ) {
			(*i)->AddSingleSplitResult(res);
			return true;
		}
	}
	// GeoSphere not found to return the data to, cancel and delete it instead
	if( res ) {
		res->OnCancel();
		delete res;
	}
	return false;
}

//static
bool GeoSphere::OnAddCPUGenResult(const SystemPath &path, CloudJobs::CloudCPUGenResult *res)
{
	// Find the correct GeoSphere via it's system path, and give it the split result
	for(auto i : s_allGeospheres) {
		if( path == i->GetSystemBody()->GetPath() ) {
			i->AddCPUGenResult(res);
			return true;
		}
	}
	// GeoSphere not found to return the data to, cancel (which deletes it) instead
	if( res ) {
		res->OnCancel();
		delete res;
	}
	return false;
}

//static
bool GeoSphere::OnAddGPUGenResult(const SystemPath &path, CloudJobs::CloudGPUGenResult *res)
{
	// Find the correct GeoSphere via it's system path, and give it the split result
	for(auto i : s_allGeospheres) {
		if( path == i->GetSystemBody()->GetPath() ) {
			i->AddGPUGenResult(res);
			return true;
		}
	}
	// GeoSphere not found to return the data to, cancel (which deletes it) instead
	if( res ) {
		res->OnCancel();
		delete res;
	}
	return false;
}
	

void GeoSphere::Reset()
{
	{
		std::deque<SSingleSplitResult*>::iterator iter = mSingleSplitResults.begin();
		while(iter!=mSingleSplitResults.end())
		{
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
		std::deque<SQuadSplitResult*>::iterator iter = mQuadSplitResults.begin();
		while(iter!=mQuadSplitResults.end())
		{
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

	for (int p=0; p<NUM_PATCHES; p++) {
		// delete patches
		if (m_patches[p]) {
			m_patches[p].reset();
		}
	}

	CalculateMaxPatchDepth();

	m_initStage = eBuildFirstPatches;
}

#define GEOSPHERE_TYPE	(GetSystemBody()->type)

GeoSphere::GeoSphere(const SystemBody *body) : BaseSphere(body),
	mCloudProcessDelay(-1), m_hasTempCampos(false), m_tempCampos(0.0), m_tempFrustum(800, 600, 0.5, 1.0, 1000.0), m_hasGpuJobRequest(false),
	m_initStage(eBuildFirstPatches), m_maxDepth(0)
{
	print_info(body, m_terrain.Get());

	for(int i=0; i<NUM_PATCHES; i++) {
		m_hasJobRequest[i] = false;
	}

	s_allGeospheres.push_back(this);

	CalculateMaxPatchDepth();

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
	assert(mQuadSplitResults.size()<MAX_SPLIT_OPERATIONS);
	if(mQuadSplitResults.size()<MAX_SPLIT_OPERATIONS) {
		mQuadSplitResults.push_back(res);
		result = true;
	}
	return result;
}

bool GeoSphere::AddSingleSplitResult(SSingleSplitResult *res)
{
	bool result = false;
	assert(res);
	assert(mSingleSplitResults.size()<MAX_SPLIT_OPERATIONS);
	if(mSingleSplitResults.size()<MAX_SPLIT_OPERATIONS) {
		mSingleSplitResults.push_back(res);
		result = true;
	}
	return result;
}

void GeoSphere::ProcessSplitResults()
{
	// now handle the single split results that define the base level of the quad tree
	{
		std::deque<SSingleSplitResult*>::iterator iter = mSingleSplitResults.begin();
		while(iter!=mSingleSplitResults.end())
		{
			// finally pass SplitResults
			SSingleSplitResult *psr = (*iter);
			assert(psr);

			const int32_t faceIdx = psr->face();
			if( m_patches[faceIdx] ) {
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
		std::deque<SQuadSplitResult*>::iterator iter = mQuadSplitResults.begin();
		while(iter!=mQuadSplitResults.end())
		{
			// finally pass SplitResults
			SQuadSplitResult *psr = (*iter);
			assert(psr);

			const int32_t faceIdx = psr->face();
			if( m_patches[faceIdx] ) {
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



bool GeoSphere::AddCPUGenResult(CloudJobs::CloudCPUGenResult *res)
{
	bool result = false;
	assert(res);
	if(res) {
		mCPUCloudResults.push_back(res);
		if(mCPUCloudResults.size() == NUM_PATCHES)
			mCloudProcessDelay = 30;
		result = true;
	}
	return result;
}

bool GeoSphere::AddGPUGenResult(CloudJobs::CloudGPUGenResult *res)
{
	bool result = false;
	assert(res);
	if(res) {
		mGPUCloudResults.push_back(res);
		mCloudProcessDelay = 30;
		result = true;
	}
	return result;
}

bool GeoSphere::ProcessCloudResults()
{
	if (mCloudProcessDelay<0) {
		return false;
	} else if (mCloudProcessDelay>0) {
		--mCloudProcessDelay;
		return false;
	} else if (mCloudProcessDelay==0) {
		mCloudProcessDelay = -1;
		ProcessCPUCloudResults();
		ProcessGPUCloudResults();
		return true;
	}
	assert(false && "how did it get here?");
	return false;
}

bool GeoSphere::ProcessCPUCloudResults()
{
	if (mCPUCloudResults.empty())
		return false;
	
	bool result = false;

	bool bCreateTexture = true;
	Sint32 uvDims = -1;
	for(int i=0; i<NUM_PATCHES; i++) {
		
		std::unique_ptr<CloudJobs::CloudCPUGenResult> res(mCPUCloudResults[i]);
		assert(res);
		assert(res->face() >= 0 && res->face() < NUM_PATCHES);
		m_jobColorBuffers[res->face()].reset( res->data().colors );
		m_hasJobRequest[res->face()] = false;
		uvDims = res->data().uvDims;
		assert( uvDims > 0 && uvDims <= 4096 );

		bCreateTexture = bCreateTexture & (m_jobColorBuffers[res->face()] != nullptr);
	}

	if( bCreateTexture ) {
		// create texture
		const vector2f texSize(1.0f, 1.0f);
		const vector2f dataSize(uvDims, uvDims);
		const Graphics::TextureDescriptor texDesc(
			Graphics::TEXTURE_RGBA_8888, 
			dataSize, texSize, Graphics::LINEAR_CLAMP, 
			true, false, false, 0, Graphics::TEXTURE_CUBE_MAP);
		m_cloudsTexture.Reset(Pi::renderer->CreateTexture(texDesc));

		// update with buffer from above
		Graphics::TextureCubeData tcd;
		tcd.posX = m_jobColorBuffers[0].get();
		tcd.negX = m_jobColorBuffers[1].get();
		tcd.posY = m_jobColorBuffers[2].get();
		tcd.negY = m_jobColorBuffers[3].get();
		tcd.posZ = m_jobColorBuffers[4].get();
		tcd.negZ = m_jobColorBuffers[5].get();
		m_cloudsTexture->Update(tcd, dataSize, Graphics::TEXTURE_RGBA_8888);

#if DUMP_TO_TEXTURE
		for (int iFace = 0; iFace<NUM_PATCHES; iFace++) {
			char filename[1024];
			snprintf(filename, 1024, "%s%d.png", GetSystemBody()->GetName().c_str(), iFace);
			textureDump(filename, uvDims, uvDims, m_jobColorBuffers[iFace].get());
		}
#endif

		// cleanup the temporary color buffer storage
		for(int i=0; i<NUM_PATCHES; i++) {
			m_jobColorBuffers[i].reset();
		}

		// change the planet texture for the new higher resolution texture
		if( m_cloudMaterial.Get() ) {
			m_cloudMaterial->texture0 = m_cloudsTexture.Get();
		}

		result = true;
	}
	mCPUCloudResults.clear();

	return result;
}

bool GeoSphere::ProcessGPUCloudResults()
{
	if (mGPUCloudResults.empty())
		return false;

	std::unique_ptr<CloudJobs::CloudGPUGenResult> res(mGPUCloudResults[0]);
	mGPUCloudResults.pop_front();
	bool result = false;
	assert(res);
	m_hasGpuJobRequest = false;
	assert(!m_gpuJob.HasJob());
	const Sint32 uvDims = res->data().uvDims;
	assert( uvDims > 0 && uvDims <= 4096 );

#if DUMP_TO_TEXTURE
	for(int iFace=0; iFace<NUM_PATCHES; iFace++) {
		std::unique_ptr<Color, FreeDeleter> buffer(static_cast<Color*>(malloc(uvDims*uvDims*4)));
		Graphics::Texture* pTex = res->data().texture.Get();
		Graphics::TextureGL* pGLTex = static_cast<Graphics::TextureGL*>(pTex);
		pGLTex->Bind();
		glGetTexImage(GL_TEXTURE_CUBE_MAP_POSITIVE_X + iFace, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.get());
		pGLTex->Unbind();

		char filename[1024];
		snprintf(filename, 1024, "%s%d.png", GetSystemBody()->GetName().c_str(), iFace);
		textureDump(filename, uvDims, uvDims, buffer.get());
	}
#endif

	if (m_builtTexture.Valid()) 
	{
		m_cloudsTexture = m_builtTexture;
		m_builtTexture.Reset();

		// these won't be automatically generated otherwise since we used it as a render target
		m_cloudsTexture->BuildMipmaps();

		// change the planet texture for the new higher resolution texture
		if( m_cloudMaterial.Get() ) {
			m_cloudMaterial->texture0 = m_cloudsTexture.Get();
		}

		result = true;
	}

	return result;
}

void GeoSphere::RequestCloudSphereTexture()
{
	const SystemBody *pSysBody = GetSystemBody();
	if(pSysBody==nullptr || SystemBody::SUPERTYPE_ROCKY_PLANET != pSysBody->GetSuperType())
		return;

	SystemBody::AtmosphereParameters atmosphere = pSysBody->CalcAtmosphereParams();
	if (m_materialParameters.atmosphere.atmosDensity <= 0.0) {
		// no atmosphere, so no clouds
		return;
	}

	for(int i=0; i<NUM_PATCHES; i++) {
		if (m_hasGpuJobRequest || m_hasJobRequest[i])
			return;
	}

	const bool bEnableGPUJobs = (Pi::config->Int("EnableGPUJobs") == 1);

	// create small texture
	if( !bEnableGPUJobs )
	{
		for(int i=0; i<NUM_PATCHES; i++) 
		{
			assert(!m_hasJobRequest[i]);
			assert(!m_job[i].HasJob());
			CloudJobs::CloudCPUGenRequest *ccgr = new CloudJobs::CloudCPUGenRequest(pSysBody->GetPath(), i, GeoSphere::OnAddCPUGenResult);
			m_job[i] = Pi::GetAsyncJobQueue()->Queue(new CloudJobs::CPUCloudSphereJob(ccgr));
			m_hasJobRequest[i] = true;
		}
	}
	else
	{
		// use m_cloudsTexture texture?
		assert(!m_hasGpuJobRequest);
		assert(!m_gpuJob.HasJob());
		
		// create texture
		const vector2f texSize(1.0f, 1.0f);
		const vector2f dataSize(CloudJobs::GetGPUTextureDimensions());
		const Graphics::TextureDescriptor texDesc(
			Graphics::TEXTURE_RGBA_8888, 
			dataSize, texSize, Graphics::LINEAR_CLAMP, 
			true, false, false, 0, Graphics::TEXTURE_CUBE_MAP);
		m_builtTexture.Reset(Pi::renderer->CreateTexture(texDesc));

		CloudJobs::CloudGPUGenRequest *pGPUReq = new CloudJobs::CloudGPUGenRequest(pSysBody->GetPath(), pSysBody->GetRadius(), m_builtTexture.Get(), GeoSphere::OnAddGPUGenResult);
		m_gpuJob = Pi::GetSyncJobQueue()->Queue(new CloudJobs::GPUCloudSphereJob(pGPUReq));
		m_hasGpuJobRequest = true;
	}
}

void GeoSphere::BuildFirstPatches()
{
	assert(!m_patches[0]);
	if(m_patches[0])
		return;

	CalculateMaxPatchDepth();

	// generate root face patches of the cube/sphere
	static const vector3d p1 = (vector3d( 1, 1, 1)).Normalized();
	static const vector3d p2 = (vector3d(-1, 1, 1)).Normalized();
	static const vector3d p3 = (vector3d(-1,-1, 1)).Normalized();
	static const vector3d p4 = (vector3d( 1,-1, 1)).Normalized();
	static const vector3d p5 = (vector3d( 1, 1,-1)).Normalized();
	static const vector3d p6 = (vector3d(-1, 1,-1)).Normalized();
	static const vector3d p7 = (vector3d(-1,-1,-1)).Normalized();
	static const vector3d p8 = (vector3d( 1,-1,-1)).Normalized();

	const uint64_t maxShiftDepth = GeoPatchID::MAX_SHIFT_DEPTH;

	m_patches[0].reset(new GeoPatch(s_patchContext, this, p1, p2, p3, p4, 0, (0ULL << maxShiftDepth)));
	m_patches[1].reset(new GeoPatch(s_patchContext, this, p4, p3, p7, p8, 0, (1ULL << maxShiftDepth)));
	m_patches[2].reset(new GeoPatch(s_patchContext, this, p1, p4, p8, p5, 0, (2ULL << maxShiftDepth)));
	m_patches[3].reset(new GeoPatch(s_patchContext, this, p2, p1, p5, p6, 0, (3ULL << maxShiftDepth)));
	m_patches[4].reset(new GeoPatch(s_patchContext, this, p3, p2, p6, p7, 0, (4ULL << maxShiftDepth)));
	m_patches[5].reset(new GeoPatch(s_patchContext, this, p8, p7, p6, p5, 0, (5ULL << maxShiftDepth)));

	for (int i=0; i<NUM_PATCHES; i++) {
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
	while (edgeMetres>gs_targetPatchTriLength && m_maxDepth<GEOPATCH_MAX_DEPTH) {
		edgeMetres *= 0.5;
		++m_maxDepth;
	}
}

void GeoSphere::Update()
{
	switch(m_initStage)
	{
	case eBuildFirstPatches:
		BuildFirstPatches();
		break;
	case eRequestedFirstPatches:
		{
			ProcessSplitResults();
			ProcessCloudResults();
			uint8_t numValidPatches = 0;
			for (int i=0; i<NUM_PATCHES; i++) {
				if(m_patches[i]->HasHeightData()) {
					++numValidPatches;
				}
			}
			m_initStage = (NUM_PATCHES==numValidPatches) ? eReceivedFirstPatches : eRequestedFirstPatches;
		} break;
	case eReceivedFirstPatches:
		{
			for (int i=0; i<NUM_PATCHES; i++) {
				m_patches[i]->NeedToUpdateVBOs();
			}
			m_initStage = eDefaultUpdateState;
		} break;
	case eDefaultUpdateState:
		if(m_hasTempCampos) {
			ProcessSplitResults();
			ProcessCloudResults();
			for (int i=0; i<NUM_PATCHES; i++) {
				m_patches[i]->LODUpdate(m_tempCampos, m_tempFrustum);
			}
			ProcessQuadSplitRequests();
		}
		break;
	}
}

void GeoSphere::AddQuadSplitRequest(double dist, SQuadSplitRequest *pReq, GeoPatch *pPatch)
{
	mQuadSplitRequests.push_back(TDistanceRequest(dist, pReq, pPatch));
}

void GeoSphere::ProcessQuadSplitRequests()
{
	class RequestDistanceSort {
	public:
		bool operator()(const TDistanceRequest &a, const TDistanceRequest &b)
		{
			return a.mDistance < b.mDistance;
		}
	};
	std::sort(mQuadSplitRequests.begin(), mQuadSplitRequests.end(), RequestDistanceSort());

	for(auto iter : mQuadSplitRequests) {
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

	if(m_initStage < eDefaultUpdateState)
		return;

	matrix4x4d trans = modelView;
	trans.Translate(-campos.x, -campos.y, -campos.z);
	renderer->SetTransform(trans); //need to set this for the following line to work
	matrix4x4d modv;
	matrix4x4d proj;
	matrix4x4ftod(renderer->GetCurrentModelView(), modv);
	matrix4x4ftod(renderer->GetCurrentProjection(), proj);
	Graphics::Frustum frustum( modv, proj );
	m_tempFrustum = frustum;

	// no frustum test of entire geosphere, since Space::Render does this
	// for each body using its GetBoundingRadius() value

	//First draw - create materials (they do not change afterwards)
	if (!m_surfaceMaterial)
		SetUpMaterials();

	bool bHasAtmosphere = false;
	{
		//Update material parameters
		//XXX no need to calculate AP every frame
		m_materialParameters.atmosphere = GetSystemBody()->CalcAtmosphereParams();
		m_materialParameters.atmosphere.center = trans * vector3d(0.0, 0.0, 0.0);
		m_materialParameters.atmosphere.planetRadius = radius;

		m_materialParameters.shadows = shadows;

		m_materialParameters.maxPatchDepth = GetMaxDepth();

		m_surfaceMaterial->specialParameter0 = &m_materialParameters;
		
		bHasAtmosphere = (m_materialParameters.atmosphere.atmosDensity > 0.0);
		if (bHasAtmosphere) {
			if(!m_atmosphereMaterial->specialParameter0) 
				RequestCloudSphereTexture();

			m_atmosphereMaterial->specialParameter0 = &m_materialParameters;

			// make atmosphere sphere slightly bigger than required so
			// that the edges of the pixel shader atmosphere jizz doesn't
			// show ugly polygonal angles
			DrawAtmosphereSurface(renderer, trans, campos,
				m_materialParameters.atmosphere.atmosRadius*1.01,
				m_atmosRenderState, m_atmosphereMaterial );
		}
	}

	// create the cloud materials, mesh and renderstate
	const float rad = m_materialParameters.atmosphere.atmosRadius * 0.99f;
	const matrix4x4d cloudsTrans(trans * matrix4x4d::ScaleMatrix(rad, rad, rad));
	if(bHasAtmosphere && m_cloudsTexture.Valid())
	{
		// bunny, ball ball!
		if( !m_cloudSphere.get() ) {
			if(!m_cloudMaterial.Valid()) {
				Graphics::MaterialDescriptor matDesc;
				matDesc.effect = Graphics::EFFECT_CLOUD_SPHERE;
				matDesc.textures = 1;
				m_cloudMaterial.Reset(Pi::renderer->CreateMaterial(matDesc));
				m_cloudMaterial->diffuse = Color4f(0.7f, 0.7f, 0.7f, 0.5f);
				m_cloudMaterial->texture0 = m_cloudsTexture.Get();
			}

			//blended
			Graphics::RenderStateDesc rsd;
			rsd.blendMode = Graphics::BLEND_ALPHA;
			rsd.depthWrite = false;
			rsd.cullMode = Graphics::CULL_NONE;
			m_cloudSphere.reset( new Graphics::Drawables::Sphere3D(Pi::renderer, m_cloudMaterial, Pi::renderer->CreateRenderState(rsd), 5, 1.0, Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL) );
		}
		m_cloudMaterial->specialParameter0 = &m_materialParameters;
	}

	Color ambient;
	Color &emission = m_surfaceMaterial->emissive;

	// save old global ambient
	const Color oldAmbient = renderer->GetAmbientColor();

	if ((GetSystemBody()->GetSuperType() == SystemBody::SUPERTYPE_STAR) || (GetSystemBody()->GetType() == SystemBody::TYPE_BROWN_DWARF)) {
		// stars should emit light and terrain should be visible from distance
		ambient.r = ambient.g = ambient.b = 51;
		ambient.a = 255;
		emission = StarSystem::starRealColors[GetSystemBody()->GetType()];
		emission.a = 255;
	}

	else {
		// give planet some ambient lighting if the viewer is close to it
		double camdist = campos.Length();
		camdist = 0.1 / (camdist*camdist);
		// why the fuck is this returning 0.1 when we are sat on the planet??
		// JJ: Because campos is relative to a unit-radius planet - 1.0 at the surface
		// XXX oh well, it is the value we want anyway...
		ambient.r = ambient.g = ambient.b = camdist * 255;
		ambient.a = 255;
	}

	renderer->SetAmbientColor(ambient);

	renderer->SetTransform(modelView);

	for (int i=0; i<NUM_PATCHES; i++) {
		m_patches[i]->Render(renderer, campos, modelView, frustum);
	}

	renderer->SetAmbientColor(oldAmbient);
	
	if( bHasAtmosphere && m_cloudSphere )
	{
		// draw it
		renderer->SetTransform(cloudsTrans);
		m_cloudSphere->Draw( Pi::renderer );
	}

	renderer->GetStats().AddToStatCount(Graphics::Stats::STAT_PLANETS, 1);
}

void GeoSphere::SetUpMaterials()
{
	//solid
	Graphics::RenderStateDesc rsd;
	m_surfRenderState = Pi::renderer->CreateRenderState(rsd);

	//blended
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	rsd.cullMode = Graphics::CULL_NONE;
	rsd.depthWrite = false;
	m_atmosRenderState = Pi::renderer->CreateRenderState(rsd);

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
		surfDesc.quality &= ~Graphics::HAS_ATMOSPHERE;
	}
	else if (GetSystemBody()->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
		//normal star
		surfDesc.lighting = false;
		surfDesc.quality &= ~Graphics::HAS_ATMOSPHERE;
		surfDesc.effect = Graphics::EFFECT_GEOSPHERE_STAR;
	} else {
		//planetoid with or without atmosphere
		const SystemBody::AtmosphereParameters ap(GetSystemBody()->CalcAtmosphereParams());
		surfDesc.lighting = true;
		if(ap.atmosDensity > 0.0) {
			surfDesc.quality |= Graphics::HAS_ATMOSPHERE;
		} else {
			surfDesc.quality &= ~Graphics::HAS_ATMOSPHERE;
		}
	}

	surfDesc.quality |= Graphics::HAS_ECLIPSES;
	const bool bEnableDetailMaps = (Pi::config->Int("DisableDetailMaps") == 0);
	if (bEnableDetailMaps) {
		surfDesc.quality |= Graphics::HAS_DETAIL_MAPS;
	}
	m_surfaceMaterial.Reset(Pi::renderer->CreateMaterial(surfDesc));

	m_texHi.Reset( Graphics::TextureBuilder::Model("textures/high.dds").GetOrCreateTexture(Pi::renderer, "model") );
	m_texLo.Reset( Graphics::TextureBuilder::Model("textures/low.dds").GetOrCreateTexture(Pi::renderer, "model") );
	m_surfaceMaterial->texture0 = m_texHi.Get();
	m_surfaceMaterial->texture1 = m_texLo.Get();

	{
		Graphics::MaterialDescriptor skyDesc;
		skyDesc.effect = Graphics::EFFECT_GEOSPHERE_SKY;
		skyDesc.lighting = true;
		skyDesc.quality |= Graphics::HAS_ECLIPSES;
		m_atmosphereMaterial.Reset(Pi::renderer->CreateMaterial(skyDesc));
		m_atmosphereMaterial->texture0 = nullptr;
		m_atmosphereMaterial->texture1 = nullptr;
	}
}
