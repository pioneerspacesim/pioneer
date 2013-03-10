// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "GeoSphere.h"
#include "GeoPatchContext.h"
#include "GeoPatch.h"
#include "perlin.h"
#include "Pi.h"
#include "RefCounted.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/VertexArray.h"
#include "graphics/gl2/GeoSphereMaterial.h"
#include "vcacheopt/vcacheopt.h"
#include <deque>
#include <algorithm>

static const int GEOPATCH_MAX_EDGELEN = 55;
int GeoSphere::s_vtxGenCount = 0;
RefCountedPtr<GeoPatchContext> GeoSphere::s_patchContext;

// must be odd numbers
static const int detail_edgeLen[5] = {
	7, 15, 25, 35, 55
};

#define PRINT_VECTOR(_v) printf("%f,%f,%f\n", (_v).x, (_v).y, (_v).z);

static const int geo_sphere_edge_friends[NUM_PATCHES][4] = {
	{ 3, 4, 1, 2 },
	{ 0, 4, 5, 2 },
	{ 0, 1, 5, 3 },
	{ 0, 2, 5, 4 },
	{ 0, 3, 5, 1 },
	{ 1, 4, 3, 2 }
};

static std::vector<GeoSphere*> s_allGeospheres;

static bool s_exitFlag = false;

void GeoSphere::Init()
{
	s_patchContext.Reset(new GeoPatchContext(detail_edgeLen[Pi::detail.planets > 4 ? 4 : Pi::detail.planets]));
	assert(s_patchContext->edgeLen <= GEOPATCH_MAX_EDGELEN);
}

void GeoSphere::Uninit()
{
	assert (s_patchContext.Unique());
	s_patchContext.Reset();
}

static void print_info(const SystemBody *sbody, const Terrain *terrain)
{
	printf(
		"%s:\n"
		"    height fractal: %s\n"
		"    colour fractal: %s\n"
		"    seed: %u\n",
		sbody->name.c_str(), terrain->GetHeightFractalName(), terrain->GetColorFractalName(), sbody->seed);
}

void GeoSphere::OnChangeDetailLevel()
{
	s_patchContext.Reset(new GeoPatchContext(detail_edgeLen[Pi::detail.planets > 4 ? 4 : Pi::detail.planets]));
	assert(s_patchContext->edgeLen <= GEOPATCH_MAX_EDGELEN);

	// reinit the geosphere terrain data
	for(std::vector<GeoSphere*>::iterator i = s_allGeospheres.begin(); i != s_allGeospheres.end(); ++i) 
	{
		for (int p=0; p<NUM_PATCHES; p++) {
			// delete patches
			if ((*i)->m_patches[p]) {
				delete (*i)->m_patches[p];
				(*i)->m_patches[p] = 0;
			}
		}

		// reinit the terrain with the new settings
		delete (*i)->m_terrain;
		(*i)->m_terrain = Terrain::InstanceTerrain((*i)->m_sbody);
		print_info((*i)->m_sbody, (*i)->m_terrain);
	}
}

#define GEOSPHERE_TYPE	(m_sbody->type)

GeoSphere::GeoSphere(const SystemBody *body) : m_sbody(body), m_terrain(Terrain::InstanceTerrain(body)), 
	mCurrentNumPatches(0), mCurrentMemAllocatedToPatches(0)
{
	print_info(body, m_terrain);

	for(int i=0; i<NUM_PATCHES; ++i) {
		m_patches[i] = NULL;
	}

	s_allGeospheres.push_back(this);

	//SetUpMaterials is not called until first Render since light count is zero :)
}

GeoSphere::~GeoSphere()
{
	// update thread should not be able to access us now, so we can safely continue to delete
	assert(std::count(s_allGeospheres.begin(), s_allGeospheres.end(), this) == 1);
	s_allGeospheres.erase(std::find(s_allGeospheres.begin(), s_allGeospheres.end(), this));

	for (int i=0; i<NUM_PATCHES; i++) if (m_patches[i]) delete m_patches[i];

	delete m_terrain;
}

/*bool GeoSphere::AddSplitRequest(SSplitRequestDescription *desc)
{
	assert(mSplitRequestDescriptions.size()<MAX_SPLIT_OPERATIONS);
	if(mSplitRequestDescriptions.size()<MAX_SPLIT_OPERATIONS) {
		mSplitRequestDescriptions.push_back(desc);
		return true;
	}
	return false;
}

void GeoSphere::ProcessSplitRequests()
{
	std::deque<SSplitRequestDescription*>::const_iterator iter = mSplitRequestDescriptions.begin();
	while (iter!=mSplitRequestDescriptions.end())
	{
		const SSplitRequestDescription* srd = (*iter);

		const vector3f v01	= (srd->v0+srd->v1).Normalized();
		const vector3f v12	= (srd->v1+srd->v2).Normalized();
		const vector3f v23	= (srd->v2+srd->v3).Normalized();
		const vector3f v30	= (srd->v3+srd->v0).Normalized();
		const vector3f cn	= (srd->centroid).Normalized();

		// 
		const vector3f vecs[4][4] = {
			{srd->v0,	v01,		cn,			v30},
			{v01,		srd->v1,	v12,		cn},
			{cn,		v12,		srd->v2,	v23},
			{v30,		cn,			v23,		srd->v3}
		};

		SSplitResult *sr = new SSplitResult(srd->patchID.GetPatchFaceIdx(), srd->depth);
		for (int i=0; i<4; i++)
		{
			//Graphics::Texture *pTex = Graphics::TextureBuilder::TerrainGen("TerrainGen").CreateTexture(Pi::renderer);
			Graphics::TextureDescriptor td(Graphics::TEXTURE_FLOAT, vector2f(sPatchContext->fboWidth(), sPatchContext->fboWidth()), Graphics::NEAREST_CLAMP, false, false);
			Graphics::Texture *pTex = Pi::renderer->CreateTexture(td);

			// render the heightmap to a framebuffer.
			mPatchGenData->v0 = vecs[i][0];
			mPatchGenData->v1 = vecs[i][1];
			mPatchGenData->v2 = vecs[i][2];
			mPatchGenData->v3 = vecs[i][3];

			Graphics::TextureGL* pTexGL = static_cast<Graphics::TextureGL*>(pTex);
			sPatchContext->renderHeightmap(0, mPatchGenData, pTexGL->GetTextureNum());

			sr->addResult(pTex, vecs[i][0], vecs[i][1], vecs[i][2], vecs[i][3], srd->patchID.NextPatchID(srd->depth+1, i));
		}

		// store result
		mSplitResult.push_back( sr );

		// cleanup after ourselves
		delete srd;

		// next!
		++iter;
	}
	mSplitRequestDescriptions.clear();
}*/

bool GeoSphere::AddSplitResult(SSplitResult *res)
{
	bool result = false;
	assert(mSplitResult.size()<MAX_SPLIT_OPERATIONS);
	if(mSplitResult.size()<MAX_SPLIT_OPERATIONS) {
		mSplitResult.push_back(res);
		result = true;
	}
	return result;
}

void GeoSphere::ProcessSplitResults()
{
	std::deque<SSplitResult*>::const_iterator iter = mSplitResult.begin();
	while(iter!=mSplitResult.end())
	{
		// finally pass SplitResults
		const SSplitResult *psr = (*iter);

		const int32_t faceIdx = psr->face;
		m_patches[faceIdx]->ReceiveHeightmaps(psr);

		// tidyup
		delete psr;

		// Next!
		++iter;
	}
	mSplitResult.clear();
}

#pragma optimize( "", off )
void GeoSphere::BuildFirstPatches()
{
	assert(NULL==m_patches[0]);
	if(NULL!=m_patches[0])
		return;

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

	m_patches[0] = new GeoPatch(s_patchContext, this, p1, p2, p3, p4, 0, (0i64 << maxShiftDepth));
	m_patches[1] = new GeoPatch(s_patchContext, this, p4, p3, p7, p8, 0, (1i64 << maxShiftDepth));
	m_patches[2] = new GeoPatch(s_patchContext, this, p1, p4, p8, p5, 0, (2i64 << maxShiftDepth));
	m_patches[3] = new GeoPatch(s_patchContext, this, p2, p1, p5, p6, 0, (3i64 << maxShiftDepth));
	m_patches[4] = new GeoPatch(s_patchContext, this, p3, p2, p6, p7, 0, (4i64 << maxShiftDepth));
	m_patches[5] = new GeoPatch(s_patchContext, this, p8, p7, p6, p5, 0, (5i64 << maxShiftDepth));
	for (int i=0; i<NUM_PATCHES; i++) {
		for (int j=0; j<4; j++) {
			m_patches[i]->edgeFriend[j] = m_patches[geo_sphere_edge_friends[i][j]];
		}
	}
	for (int i=0; i<NUM_PATCHES; i++) {
		m_patches[i]->vertices = new vector3d[s_patchContext->NUMVERTICES()];
		m_patches[i]->normals = new vector3d[s_patchContext->NUMVERTICES()];
		m_patches[i]->colors = new vector3d[s_patchContext->NUMVERTICES()];
	}
	for (int i=0; i<NUM_PATCHES; i++) m_patches[i]->GenerateMesh();
	for (int i=0; i<NUM_PATCHES; i++) m_patches[i]->GenerateEdgeNormalsAndColors();
	for (int i=0; i<NUM_PATCHES; i++) m_patches[i]->UpdateVBOs();
}

static const float g_ambient[4] = { 0, 0, 0, 1.0 };

static void DrawAtmosphereSurface(Graphics::Renderer *renderer,
	const vector3d &campos, float rad, Graphics::Material *mat)
{
	const int LAT_SEGS = 20;
	const int LONG_SEGS = 20;
	vector3d yaxis = campos.Normalized();
	vector3d zaxis = vector3d(1.0,0.0,0.0).Cross(yaxis).Normalized();
	vector3d xaxis = yaxis.Cross(zaxis);
	const matrix4x4d m = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();

	glPushMatrix();
	glScalef(rad, rad, rad);
	glMultMatrixd(&m[0]);

	// what is this? Well, angle to the horizon is:
	// acos(planetRadius/viewerDistFromSphereCentre)
	// and angle from this tangent on to atmosphere is:
	// acos(planetRadius/atmosphereRadius) ie acos(1.0/1.01244blah)
	double endAng = acos(1.0/campos.Length())+acos(1.0/rad);
	double latDiff = endAng / double(LAT_SEGS);

	double rot = 0.0;
	float sinCosTable[LONG_SEGS+1][2];
	for (int i=0; i<=LONG_SEGS; i++, rot += 2.0*M_PI/double(LONG_SEGS)) {
		sinCosTable[i][0] = float(sin(rot));
		sinCosTable[i][1] = float(cos(rot));
	}

	/* Tri-fan above viewer */
	Graphics::VertexArray va(Graphics::ATTRIB_POSITION);
	va.Add(vector3f(0.f, 1.f, 0.f));
	for (int i=0; i<=LONG_SEGS; i++) {
		va.Add(vector3f(
			sin(latDiff)*sinCosTable[i][0],
			cos(latDiff),
			-sin(latDiff)*sinCosTable[i][1]));
	}
	renderer->DrawTriangles(&va, mat, Graphics::TRIANGLE_FAN);

	/* and wound latitudinal strips */
	double lat = latDiff;
	for (int j=1; j<LAT_SEGS; j++, lat += latDiff) {
		Graphics::VertexArray v(Graphics::ATTRIB_POSITION);
		float cosLat = cos(lat);
		float sinLat = sin(lat);
		float cosLat2 = cos(lat+latDiff);
		float sinLat2 = sin(lat+latDiff);
		for (int i=0; i<=LONG_SEGS; i++) {
			v.Add(vector3f(sinLat*sinCosTable[i][0], cosLat, -sinLat*sinCosTable[i][1]));
			v.Add(vector3f(sinLat2*sinCosTable[i][0], cosLat2, -sinLat2*sinCosTable[i][1]));
		}
		renderer->DrawTriangles(&v, mat, Graphics::TRIANGLE_STRIP);
	}

	glPopMatrix();
}

#pragma optimize( "", off )
void GeoSphere::Update()
{
	if(NULL==m_patches[0] && mSplitResult.empty()) {
		BuildFirstPatches();
	} else /*if(mSplitRequestDescriptions.empty())*/ {
		ProcessSplitResults();
		for (int i=0; i<NUM_PATCHES; i++) {
			m_patches[i]->LODUpdate(m_tempCampos);
		}
	}
}

void GeoSphere::Render(Graphics::Renderer *renderer, vector3d campos, const float radius, const float scale) 
{
	if(NULL==m_patches[0]) {
		return;
	}

	glPushMatrix();
	glTranslated(-campos.x, -campos.y, -campos.z);
	Graphics::Frustum frustum = Graphics::Frustum::FromGLState();

	// no frustum test of entire geosphere, since Space::Render does this
	// for each body using its GetBoundingRadius() value

	//First draw - create materials (they do not change afterwards)
	if (!m_surfaceMaterial.Valid())
		SetUpMaterials();

	if (Graphics::AreShadersEnabled()) {
		matrix4x4d modelMatrix;
		glGetDoublev (GL_MODELVIEW_MATRIX, &modelMatrix[0]);

		//Update material parameters
		//XXX no need to calculate AP every frame
		m_atmosphereParameters = m_sbody->CalcAtmosphereParams();
		m_atmosphereParameters.center = modelMatrix * vector3d(0.0, 0.0, 0.0);
		m_atmosphereParameters.planetRadius = radius;
		m_atmosphereParameters.scale = scale;

		m_surfaceMaterial->specialParameter0 = &m_atmosphereParameters;

		if (m_atmosphereParameters.atmosDensity > 0.0) {
			m_atmosphereMaterial->specialParameter0 = &m_atmosphereParameters;

			renderer->SetBlendMode(Graphics::BLEND_ALPHA_ONE);
			renderer->SetDepthWrite(false);
			// make atmosphere sphere slightly bigger than required so
			// that the edges of the pixel shader atmosphere jizz doesn't
			// show ugly polygonal angles
			DrawAtmosphereSurface(renderer, campos, m_atmosphereParameters.atmosRadius*1.01, m_atmosphereMaterial.Get());
			renderer->SetDepthWrite(true);
			renderer->SetBlendMode(Graphics::BLEND_SOLID);
		}
	}
	glPopMatrix();

	Color ambient;
	Color &emission = m_surfaceMaterial->emissive;

	// save old global ambient
	const Color oldAmbient = renderer->GetAmbientColor();

	if ((m_sbody->GetSuperType() == SystemBody::SUPERTYPE_STAR) || (m_sbody->type == SystemBody::TYPE_BROWN_DWARF)) {
		// stars should emit light and terrain should be visible from distance
		ambient.r = ambient.g = ambient.b = 0.2f;
		ambient.a = 1.0f;
		emission.r = StarSystem::starRealColors[m_sbody->type][0];
		emission.g = StarSystem::starRealColors[m_sbody->type][1];
		emission.b = StarSystem::starRealColors[m_sbody->type][2];
		emission.a = 1.f;
	}

	else {
		// give planet some ambient lighting if the viewer is close to it
		double camdist = campos.Length();
		camdist = 0.1 / (camdist*camdist);
		// why the fuck is this returning 0.1 when we are sat on the planet??
		// JJ: Because campos is relative to a unit-radius planet - 1.0 at the surface
		// XXX oh well, it is the value we want anyway...
		ambient.r = ambient.g = ambient.b = float(camdist);
		ambient.a = 1.0f;
	}

	renderer->SetAmbientColor(ambient);
//#define USE_WIREFRAME
#ifdef USE_WIREFRAME
	renderer->SetWireFrameMode(true);
#endif
	// this is pretty much the only place where a non-renderer is allowed to call Apply()
	// to be removed when someone rewrites terrain
	m_surfaceMaterial->Apply();

	for (int i=0; i<NUM_PATCHES; i++) {
		m_patches[i]->Render(campos, frustum);
	}

	m_surfaceMaterial->Unapply();

	renderer->SetAmbientColor(oldAmbient);
#ifdef USE_WIREFRAME
	renderer->SetWireFrameMode(false);
#endif

	// store this for later usage in the update method.
	m_tempCampos = campos;
}

void GeoSphere::SetUpMaterials()
{
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

	if ((m_sbody->type == SystemBody::TYPE_BROWN_DWARF) ||
		(m_sbody->type == SystemBody::TYPE_STAR_M)) {
		//dim star (emits and receives light)
		surfDesc.lighting = true;
		surfDesc.atmosphere = false;
	}
	else if (m_sbody->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
		//normal star
		surfDesc.lighting = false;
		surfDesc.atmosphere = false;
	} else {
		//planetoid with or without atmosphere
		const SystemBody::AtmosphereParameters ap(m_sbody->CalcAtmosphereParams());
		surfDesc.lighting = true;
		surfDesc.atmosphere = (ap.atmosDensity > 0.0);
	}
	m_surfaceMaterial.Reset(Pi::renderer->CreateMaterial(surfDesc));

	//Shader-less atmosphere is drawn in Planet
	if (Graphics::AreShadersEnabled()) {
		Graphics::MaterialDescriptor skyDesc;
		skyDesc.effect = Graphics::EFFECT_GEOSPHERE_SKY;
		skyDesc.lighting = true;
		m_atmosphereMaterial.Reset(Pi::renderer->CreateMaterial(skyDesc));
	}
}
