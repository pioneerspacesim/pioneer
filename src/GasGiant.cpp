// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "GasGiant.h"
#include "perlin.h"
#include "Pi.h"
#include "Game.h"
#include "RefCounted.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/TextureBuilder.h"
#include "graphics/VertexArray.h"
#include "vcacheopt/vcacheopt.h"
#include <deque>
#include <algorithm>

RefCountedPtr<GasPatchContext> GasGiant::s_patchContext;

namespace
{
	static const float s_initialDelayTime = 60.0f; // (perhaps) 60 seconds seems like a reasonable default
	static std::vector<GasGiant*> s_allGasGiants;

	// generate root face patches of the cube/sphere
	static const vector3d p1 = (vector3d( 1, 1, 1)).Normalized();
	static const vector3d p2 = (vector3d(-1, 1, 1)).Normalized();
	static const vector3d p3 = (vector3d(-1,-1, 1)).Normalized();
	static const vector3d p4 = (vector3d( 1,-1, 1)).Normalized();
	static const vector3d p5 = (vector3d( 1, 1,-1)).Normalized();
	static const vector3d p6 = (vector3d(-1, 1,-1)).Normalized();
	static const vector3d p7 = (vector3d(-1,-1,-1)).Normalized();
	static const vector3d p8 = (vector3d( 1,-1,-1)).Normalized();

	class STextureFaceRequest {
	public:
		STextureFaceRequest(const vector3d *v_, const SystemPath &sysPath_, const Sint32 face_, const Sint32 uvDIMs_, Terrain *pTerrain_) :
			corners(v_), sysPath(sysPath_), face(face_), uvDIMs(uvDIMs_), pTerrain(pTerrain_)
		{
			colors = new Color[NumTexels()];
		}

		 // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
		// Use only data local to this object
		void OnRun()
		{
			assert( corners != nullptr );
			double fracStep = 1.0 / double(UVDims()-1);
			for( Sint32 v=0; v<UVDims(); v++ ) {
				for( Sint32 u=0; u<UVDims(); u++ ) {
					// where in this row & colum are we now.
					const double ustep = double(u) * fracStep;
					const double vstep = double(v) * fracStep;

					// get point on the surface of the sphere
					const vector3d p = GetSpherePoint(ustep, vstep);
					// get colour using `p`
					const vector3d colour = pTerrain->GetColor(p, 0.0, p);

					// convert to ubyte and store
					Color* col = colors + (u + (v * UVDims()));
					col[0].r = Uint8(colour.x * 255.0);
					col[0].g = Uint8(colour.y * 255.0);
					col[0].b = Uint8(colour.z * 255.0);
					col[0].a = 255;
				}
			}
		}

		Sint32 Face() const { return face; }
		inline Sint32 UVDims() const { return uvDIMs; }
		Color* Colors() const { return colors; }
		const SystemPath& SysPath() const { return sysPath; }

	protected:
		// deliberately prevent copy constructor access
		STextureFaceRequest(const STextureFaceRequest &r);

		inline Sint32 NumTexels() const { return uvDIMs*uvDIMs; }

		// in patch surface coords, [0,1]
		inline vector3d GetSpherePoint(const double x, const double y) const {
			return (corners[0] + x*(1.0-y)*(corners[1]-corners[0]) + x*y*(corners[2]-corners[0]) + (1.0-x)*y*(corners[3]-corners[0])).Normalized();
		}

		// these are created with the request and are given to the resulting patches
		Color *colors;

		const vector3d *corners;
		const SystemPath sysPath;
		const Sint32 face;
		const Sint32 uvDIMs;
		RefCountedPtr<Terrain> pTerrain;
	};

	class STextureFaceResult {
	public:
		struct STextureFaceData {
			STextureFaceData() {}
			STextureFaceData(Color *c_, Sint32 uvDims_) : colors(c_), uvDims(uvDims_) {}
			STextureFaceData(const STextureFaceData &r) : colors(r.colors), uvDims(r.uvDims) {}
			Color *colors;
			Sint32 uvDims;
		};

		STextureFaceResult(const int32_t face_) : mFace(face_) {}

		void addResult(Color *c_, Sint32 uvDims_) {
			mData = STextureFaceData(c_, uvDims_);
		}

		inline const STextureFaceData& data() const { return mData; }
		inline int32_t face() const { return mFace; }

		void OnCancel()	{
			if( mData.colors ) {delete [] mData.colors; mData.colors = NULL;}
		}

	protected:
		// deliberately prevent copy constructor access
		STextureFaceResult(const STextureFaceResult &r);

		const int32_t mFace;
		STextureFaceData mData;
	};

	// ********************************************************************************
	// Overloaded PureJob class to handle generating the mesh for each patch
	// ********************************************************************************
	class SingleTextureFaceJob : public Job
	{
	public:
		SingleTextureFaceJob(STextureFaceRequest *data) : mData(data), mpResults(nullptr) { /* empty */ }
		virtual ~SingleTextureFaceJob()
		{
			if(mpResults) {
				mpResults->OnCancel();
				delete mpResults;
				mpResults = nullptr;
			}
		}

		virtual void OnRun() // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
		{
			mData->OnRun();

			// add this patches data
			STextureFaceResult *sr = new STextureFaceResult(mData->Face());
			sr->addResult(mData->Colors(), mData->UVDims());

			// store the result
			mpResults = sr;
		}
		virtual void OnFinish() // runs in primary thread of the context
		{
			GasGiant::OnAddTextureFaceResult( mData->SysPath(), mpResults );
			mpResults = nullptr;
		}
		virtual void OnCancel() {}

	private:
		// deliberately prevent copy constructor access
		SingleTextureFaceJob(const SingleTextureFaceJob &r);

		std::unique_ptr<STextureFaceRequest> mData;
		STextureFaceResult *mpResults;
	};
}


class GasPatchContext : public RefCounted {
public:
	#pragma pack(push, 4)
	struct VBOVertex
	{
		vector3f pos;
		vector3f norm;
	};
	#pragma pack(pop)

	int edgeLen;

	inline int IDX_VBO_COUNT_ALL_IDX()	const { return ((edgeLen-1)*(edgeLen-1))*2*3; }

	inline int NUMVERTICES() const { return edgeLen*edgeLen; }

	double frac;

	std::unique_ptr<unsigned short[]> indices;
	RefCountedPtr<Graphics::IndexBuffer> indexBuffer;

	GasPatchContext(const int _edgeLen) : edgeLen(_edgeLen) {
		Init();
	}

	~GasPatchContext() {
		Cleanup();
	}

	void Refresh() {
		Cleanup();
		Init();
	}

	void Cleanup() {
		indices.reset();
	}

	int getIndices(std::vector<unsigned short> &pl)
	{
		// calculate how many tri's there are
		const int tri_count = IDX_VBO_COUNT_ALL_IDX()/3;

		// pre-allocate enough space
		pl.reserve(IDX_VBO_COUNT_ALL_IDX());

		// add all of the middle indices
		for(int i=0; i<IDX_VBO_COUNT_ALL_IDX(); ++i) {
			pl.push_back(indices[i]);
		}

		return tri_count;
	}
	
	void Init() {
		PROFILE_SCOPED()
		frac = 1.0 / double(edgeLen-1);

		// also want vtx indices for tris not touching edge of patch 
		indices.reset(new unsigned short[IDX_VBO_COUNT_ALL_IDX()]);
		unsigned short *idx = indices.get();
		for (int x=0; x<edgeLen-1; x++) {
			for (int y=0; y<edgeLen-1; y++) {
				idx[0] = x + edgeLen*y;
				idx[1] = x+1 + edgeLen*y;
				idx[2] = x + edgeLen*(y+1);
				idx+=3;

				idx[0] = x+1 + edgeLen*y;
				idx[1] = x+1 + edgeLen*(y+1);
				idx[2] = x + edgeLen*(y+1);
				idx+=3;
			}
		}

		// these will hold the optimised indices
		std::vector<unsigned short> pl_short;

		// populate the N indices lists from the arrays built during InitTerrainIndices()
		// iterate over each index list and optimize it
		unsigned int tri_count = getIndices(pl_short);
		VertexCacheOptimizerUShort vco;
		VertexCacheOptimizerUShort::Result res = vco.Optimize(&pl_short[0], tri_count);
		assert(0 == res);

		//create buffer & copy
		indexBuffer.Reset(Pi::renderer->CreateIndexBuffer(pl_short.size(), Graphics::BUFFER_USAGE_STATIC));
		Uint16* idxPtr = indexBuffer->Map(Graphics::BUFFER_MAP_WRITE);
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
	std::unique_ptr<Graphics::VertexBuffer> m_vertexBuffer;
	GasGiant *gasSphere;
	vector3d clipCentroid;
	double clipRadius;

	GasPatch(const RefCountedPtr<GasPatchContext> &_ctx, GasGiant *gs, vector3d v0, vector3d v1, vector3d v2, vector3d v3) 
		: ctx(_ctx), gasSphere(gs), clipCentroid(((v0+v1+v2+v3) * 0.25).Normalized()), clipRadius(0.0)
	{
		PROFILE_SCOPED()
		v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3;
		for (int i=0; i<4; i++) {
			clipRadius = std::max(clipRadius, (v[i]-clipCentroid).Length());
		}

		UpdateVBOs();
	}

	~GasPatch() {}

	/* in patch surface coords, [0,1] */
	vector3d GetSpherePoint(const double x, const double y) const {
		return (v[0] + x*(1.0-y)*(v[1]-v[0]) + x*y*(v[2]-v[0]) + (1.0-x)*y*(v[3]-v[0])).Normalized();
	}

	void UpdateVBOs() {
		PROFILE_SCOPED()
		//create buffer and upload data
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[1].semantic = Graphics::ATTRIB_NORMAL;
		vbd.attrib[1].format   = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.numVertices = ctx->NUMVERTICES();
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;
		m_vertexBuffer.reset(Pi::renderer->CreateVertexBuffer(vbd));

		GasPatchContext::VBOVertex* vtxPtr = m_vertexBuffer->Map<GasPatchContext::VBOVertex>(Graphics::BUFFER_MAP_WRITE);
		assert(m_vertexBuffer->GetDesc().stride == sizeof(GasPatchContext::VBOVertex));
		
		const Sint32 edgeLen = ctx->edgeLen;
		const double frac = ctx->frac;
		for (Sint32 y=0; y<edgeLen; y++) {
			for (Sint32 x=0; x<edgeLen; x++) {
				const vector3d p = GetSpherePoint(x*frac, y*frac);
				const vector3d pSubCentroid = p - clipCentroid;
				clipRadius = std::max(clipRadius, p.Length());
				vtxPtr->pos = vector3f(pSubCentroid);
				vtxPtr->norm = vector3f(p);

				++vtxPtr; // next vertex
			}
		}
		m_vertexBuffer->Unmap();
	}

	void Render(Graphics::Renderer *renderer, const vector3d &campos, const matrix4x4d &modelView, const Graphics::Frustum &frustum) {
		if (!frustum.TestPoint(clipCentroid, clipRadius))
			return;

		Graphics::Material *mat = gasSphere->GetSurfaceMaterial();
		Graphics::RenderState *rs = gasSphere->GetSurfRenderState();

		const vector3d relpos = clipCentroid - campos;
		renderer->SetTransform(modelView * matrix4x4d::Translation(relpos));

		Pi::statSceneTris += 2*(ctx->edgeLen-1)*(ctx->edgeLen-1);

		renderer->DrawBufferIndexed(m_vertexBuffer.get(), ctx->indexBuffer.Get(), rs, mat);
	}
};

// static
void GasGiant::UpdateAllGasGiants()
{
	PROFILE_SCOPED()
	for(std::vector<GasGiant*>::iterator i = s_allGasGiants.begin(); i != s_allGasGiants.end(); ++i)
	{
		(*i)->Update();
	}
}

GasGiant::GasGiant(const SystemBody *body) : BaseSphere(body),
	m_hasTempCampos(false), m_tempCampos(0.0), m_timeDelay(s_initialDelayTime)
{
	s_allGasGiants.push_back(this);
	
	for(int i=0; i<NUM_PATCHES; i++) 
		m_hasJobRequest[i] = false;

	//SetUpMaterials is not called until first Render since light count is zero :)

	//BuildFirstPatches and GenerateTexture are only called when we first attempt to render
}

GasGiant::~GasGiant()
{
	// update thread should not be able to access us now, so we can safely continue to delete
	assert(std::count(s_allGasGiants.begin(), s_allGasGiants.end(), this) == 1);
	s_allGasGiants.erase(std::find(s_allGasGiants.begin(), s_allGasGiants.end(), this));
}

//static
bool GasGiant::OnAddTextureFaceResult(const SystemPath &path, STextureFaceResult *res)
{
	// Find the correct GeoSphere via it's system path, and give it the split result
	for(std::vector<GasGiant*>::iterator i=s_allGasGiants.begin(), iEnd=s_allGasGiants.end(); i!=iEnd; ++i) {
		if( path == (*i)->GetSystemBody()->GetPath() ) {
			(*i)->AddTextureFaceResult(res);
			return true;
		}
	}
	// GasGiant not found to return the data to, cancel (which deletes it) instead
	if( res ) {
		res->OnCancel();
		delete res;
	}
	return false;
}

bool GasGiant::AddTextureFaceResult(STextureFaceResult *res)
{
	bool result = false;
	assert(res);
	assert(res->face() >= 0 && res->face() < NUM_PATCHES);
	m_jobColorBuffers[res->face()].reset( res->data().colors );
	m_hasJobRequest[res->face()] = false;
	const Sint32 uvDims = res->data().uvDims;
	assert( uvDims > 0 && uvDims <= 4096 );

	// tidyup
	delete res;

	bool bCreateTexture = true;
	for(int i=0; i<NUM_PATCHES; i++) {
		bCreateTexture = bCreateTexture & (!m_hasJobRequest[i]);
	}

	if( bCreateTexture ) {
		// create texture
		const vector2f texSize(1.0f, 1.0f);
		const vector2f dataSize(uvDims, uvDims);
		const Graphics::TextureDescriptor texDesc(
			Graphics::TEXTURE_RGBA_8888, 
			dataSize, texSize, Graphics::LINEAR_CLAMP, 
			true, false, 0, Graphics::TEXTURE_CUBE_MAP);
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
		for(int i=0; i<NUM_PATCHES; i++) {
			m_jobColorBuffers[i].reset();
		}

		// change the planet texture for the new higher resolution texture
		if( m_surfaceMaterial.get() ) {
			m_surfaceMaterial->texture0 = m_surfaceTexture.Get();
		}
	}

	return result;
}

static const Uint32 UV_DIMS_SMALL = 16;
static const Uint32 UV_DIMS = 512;

static const vector3d s_patchFaces[NUM_PATCHES][4] = 
{ 
	{p5, p1, p4, p8}, // +x
	{p2, p6, p7, p3}, // -x
	
	{p2, p1, p5, p6}, // +y
	{p7, p8, p4, p3}, // -y

	{p6, p5, p8, p7}, // +z - NB: these are actually reversed!
	{p1, p2, p3, p4}  // -z
};

// in patch surface coords, [0,1]
inline vector3d GetSpherePointFromCorners(const double x, const double y, const vector3d *corners) {
	return (corners[0] + x*(1.0-y)*(corners[1]-corners[0]) + x*y*(corners[2]-corners[0]) + (1.0-x)*y*(corners[3]-corners[0])).Normalized();
}

void GasGiant::GenerateTexture()
{
	// create small texture
	{
		const vector2f texSize(1.0f, 1.0f);
		const vector2f dataSize(UV_DIMS_SMALL, UV_DIMS_SMALL);
		const Graphics::TextureDescriptor texDesc(
			Graphics::TEXTURE_RGBA_8888, 
			dataSize, texSize, Graphics::LINEAR_CLAMP, 
			false, false, 0, Graphics::TEXTURE_CUBE_MAP);
		m_surfaceTextureSmall.Reset(Pi::renderer->CreateTexture(texDesc));

		const Terrain *pTerrain = GetTerrain();
		const double fracStep = 1.0 / double(UV_DIMS_SMALL-1);

		Graphics::TextureCubeData tcd;
		std::unique_ptr<Color> bufs[NUM_PATCHES];
		for(int i=0; i<NUM_PATCHES; i++) {
			Color *colors = new Color[ (UV_DIMS_SMALL*UV_DIMS_SMALL) ];
			for( Uint32 v=0; v<UV_DIMS_SMALL; v++ ) {
				for( Uint32 u=0; u<UV_DIMS_SMALL; u++ ) {
					// where in this row & colum are we now.
					const double ustep = double(u) * fracStep;
					const double vstep = double(v) * fracStep;

					// get point on the surface of the sphere
					const vector3d p = GetSpherePointFromCorners(ustep, vstep, &s_patchFaces[i][0]);
					// get colour using `p`
					const vector3d colour = pTerrain->GetColor(p, 0.0, p);

					// convert to ubyte and store
					Color* col = colors + (u + (v * UV_DIMS_SMALL));
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
	
	for(int i=0; i<NUM_PATCHES; i++) 
	{
		assert(!m_hasJobRequest[i]);
		assert(!m_job[i].HasJob());
		m_hasJobRequest[i] = true;
		STextureFaceRequest *ssrd = new STextureFaceRequest(&s_patchFaces[i][0], GetSystemBody()->GetPath(), i, UV_DIMS, GetTerrain());
		m_job[i] = Pi::GetAsyncJobQueue()->Queue(new SingleTextureFaceJob(ssrd));
	}
}

void GasGiant::Update()
{
	PROFILE_SCOPED()
	// assuming that we haven't already generated the texture from the render call.
	if( m_timeDelay > 0.0f )
	{
		m_timeDelay -= Pi::game->GetTimeStep();
		if( m_timeDelay <= 0.0001f && !m_surfaceTexture.Valid() )
		{
			// Use the fact that we have a patch as a latch to prevent repeat generation requests.
			if( m_patches[0].get() )
				return;

			BuildFirstPatches();
			return;
		}
	}
}

void GasGiant::Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const float scale, const std::vector<Camera::Shadow> &shadows)
{
	if( !m_surfaceTexture.Valid() )
	{
		// Use the fact that we have a patch as a latch to prevent repeat generation requests.
		if( !m_patches[0].get() )
		{
			BuildFirstPatches();
		}
	}

	// store this for later usage in the update method.
	m_tempCampos = campos;
	m_hasTempCampos = true;

	matrix4x4d trans = modelView;
	trans.Translate(-campos.x, -campos.y, -campos.z);
	renderer->SetTransform(trans); //need to set this for the following line to work
	matrix4x4d modv;
	matrix4x4d proj;
	matrix4x4ftod(renderer->GetCurrentModelView(), modv);
	matrix4x4ftod(renderer->GetCurrentProjection(), proj);
	Graphics::Frustum frustum( modv, proj );

	// no frustum test of entire gasSphere, since Space::Render does this
	// for each body using its GetBoundingRadius() value

	//First draw - create materials (they do not change afterwards)
	if (!m_surfaceMaterial)
		SetUpMaterials();

	{
		//Update material parameters
		//XXX no need to calculate AP every frame
		m_materialParameters.atmosphere = GetSystemBody()->CalcAtmosphereParams();
		m_materialParameters.atmosphere.center = trans * vector3d(0.0, 0.0, 0.0);
		m_materialParameters.atmosphere.planetRadius = radius;
		m_materialParameters.atmosphere.scale = scale;

		m_materialParameters.shadows = shadows;

		m_surfaceMaterial->specialParameter0 = &m_materialParameters;

		if (m_materialParameters.atmosphere.atmosDensity > 0.0) {
			m_atmosphereMaterial->specialParameter0 = &m_materialParameters;

			// make atmosphere sphere slightly bigger than required so
			// that the edges of the pixel shader atmosphere jizz doesn't
			// show ugly polygonal angles
			DrawAtmosphereSurface(renderer, trans, campos, m_materialParameters.atmosphere.atmosRadius*1.01, m_atmosRenderState, m_atmosphereMaterial.get());
		}
	}

	Color ambient;

	// save old global ambient
	const Color oldAmbient = renderer->GetAmbientColor();

	{
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

	m_surfaceMaterial->Unapply();

	renderer->SetAmbientColor(oldAmbient);
}

void GasGiant::SetUpMaterials()
{
	//solid
	Graphics::RenderStateDesc rsd;
	m_surfRenderState = Pi::renderer->CreateRenderState(rsd);

	//blended
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	rsd.depthWrite = false;
	m_atmosRenderState = Pi::renderer->CreateRenderState(rsd);

	// Request material for this planet, with atmosphere. 
	// Separate materials for surface and sky.
	Graphics::MaterialDescriptor surfDesc;
	surfDesc.effect = Graphics::EFFECT_GASSPHERE_TERRAIN;

	//planetoid with atmosphere
	const SystemBody::AtmosphereParameters ap(GetSystemBody()->CalcAtmosphereParams());
	surfDesc.lighting = true;
	assert(ap.atmosDensity > 0.0);
	{
		surfDesc.quality |= Graphics::HAS_ATMOSPHERE;
	} 

	const bool bEnableEclipse = (Pi::config->Int("DisableEclipse") == 0);
	if (bEnableEclipse) {
		surfDesc.quality |= Graphics::HAS_ECLIPSES;
	}
	surfDesc.textures = 1;
	m_surfaceMaterial.reset(Pi::renderer->CreateMaterial(surfDesc));
	assert(m_surfaceTextureSmall.Valid());
	m_surfaceMaterial->texture0 = m_surfaceTexture.Valid() ? m_surfaceTexture.Get() : m_surfaceTextureSmall.Get();

	{
		Graphics::MaterialDescriptor skyDesc;
		skyDesc.effect = Graphics::EFFECT_GEOSPHERE_SKY;
		skyDesc.lighting = true;
		if (bEnableEclipse) {
			skyDesc.quality |= Graphics::HAS_ECLIPSES;
		}
		m_atmosphereMaterial.reset(Pi::renderer->CreateMaterial(skyDesc));
	}
}

void GasGiant::BuildFirstPatches()
{
	PROFILE_SCOPED()
	if( s_patchContext.Get() == nullptr ) {
		s_patchContext.Reset(new GasPatchContext(127));
	}

	m_patches[0].reset(new GasPatch(s_patchContext, this, p1, p2, p3, p4));
	m_patches[1].reset(new GasPatch(s_patchContext, this, p4, p3, p7, p8));
	m_patches[2].reset(new GasPatch(s_patchContext, this, p1, p4, p8, p5));
	m_patches[3].reset(new GasPatch(s_patchContext, this, p2, p1, p5, p6));
	m_patches[4].reset(new GasPatch(s_patchContext, this, p3, p2, p6, p7));
	m_patches[5].reset(new GasPatch(s_patchContext, this, p8, p7, p6, p5));

	GenerateTexture();
}
