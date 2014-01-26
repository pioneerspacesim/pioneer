// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "GasGiant.h"
#include "perlin.h"
#include "Pi.h"
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

static void print_info(const SystemBody *sbody, const Terrain *terrain)
{
	printf(
		"%s:\n"
		"    height fractal: %s\n"
		"    colour fractal: %s\n"
		"    seed: %u\n",
		sbody->name.c_str(), terrain->GetHeightFractalName(), terrain->GetColorFractalName(), sbody->seed);
}

#pragma pack(4)
struct VBOVertex
{
	float x,y,z;
	float nx,ny,nz;
	float padding[2];
};
#pragma pack()

class GasPatchContext : public RefCounted {
public:
	int edgeLen;

	inline int IDX_VBO_COUNT_ALL_IDX()	const { return ((edgeLen-1)*(edgeLen-1))*2*3; }

	inline int NUMVERTICES() const { return edgeLen*edgeLen; }

	double frac;

	std::unique_ptr<unsigned short[]> indices;
	std::unique_ptr<VBOVertex> vbotemp;
	GLuint indices_vbo;
	GLuint indices_tri_count;

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
		if (indices_vbo) {
			glDeleteBuffersARB(1, &indices_vbo);
		}
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
		frac = 1.0 / double(edgeLen-1);

		vbotemp.reset( new VBOVertex[NUMVERTICES()] );

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
		indices_tri_count = getIndices(pl_short);
		VertexCacheOptimizerUShort vco;
		VertexCacheOptimizerUShort::Result res = vco.Optimize(&pl_short[0], indices_tri_count);
		assert(0 == res);

		// everything should be hunky-dory for setting up as OpenGL index buffers now.
		glGenBuffersARB(1, &indices_vbo);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*indices_tri_count*3, &(pl_short[0]), GL_STATIC_DRAW);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);

		if (indices) {
			indices.reset();
		}
	}
};


class GasPatch {
public:
	RefCountedPtr<GasPatchContext> ctx;
	vector3d v[4];
	GLuint m_vbo;
	GasGiant *gasSphere;
	vector3d clipCentroid;
	double clipRadius;

	GasPatch(const RefCountedPtr<GasPatchContext> &_ctx, GasGiant *gs, vector3d v0, vector3d v1, vector3d v2, vector3d v3) 
		: ctx(_ctx), m_vbo(0), gasSphere(gs), clipCentroid(((v0+v1+v2+v3) * 0.25).Normalized()), clipRadius(0.0)
	{
		v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3;
		for (int i=0; i<4; i++) {
			clipRadius = std::max(clipRadius, (v[i]-clipCentroid).Length());
		}

		UpdateVBOs();
	}

	~GasPatch() {
		glDeleteBuffersARB(1, &m_vbo);
	}

	/* in patch surface coords, [0,1] */
	vector3d GetSpherePoint(const double x, const double y) const {
		return (v[0] + x*(1.0-y)*(v[1]-v[0]) + x*y*(v[2]-v[0]) + (1.0-x)*y*(v[3]-v[0])).Normalized();
	}

	void UpdateVBOs() {
		if (!m_vbo) 
			glGenBuffersARB(1, &m_vbo);
		
		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(VBOVertex)*ctx->NUMVERTICES(), 0, GL_DYNAMIC_DRAW);
		VBOVertex *pData = ctx->vbotemp.get();
		for (int y=0; y<ctx->edgeLen; y++) {
			for (int x=0; x<ctx->edgeLen; x++) {
				const vector3d p = GetSpherePoint(x*ctx->frac, y*ctx->frac);
				const vector3d pSubCentroid = p - clipCentroid;
				clipRadius = std::max(clipRadius, p.Length());
				pData->x = float(pSubCentroid.x);
				pData->y = float(pSubCentroid.y);
				pData->z = float(pSubCentroid.z);

				pData->nx = float(p.x);
				pData->ny = float(p.y);
				pData->nz = float(p.z);

				++pData; // next vertex
			}
		}
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(VBOVertex)*ctx->NUMVERTICES(), ctx->vbotemp.get(), GL_DYNAMIC_DRAW);
		glBindBufferARB(GL_ARRAY_BUFFER, 0);
	}

	void Render(Graphics::Renderer *renderer, const vector3d &campos, const matrix4x4d &modelView, const Graphics::Frustum &frustum) {
		if (!frustum.TestPoint(clipCentroid, clipRadius))
			return;

		vector3d relpos = clipCentroid - campos;
		glPushMatrix();
		glTranslated(relpos.x, relpos.y, relpos.z);

		Pi::statSceneTris += 2*(ctx->edgeLen-1)*(ctx->edgeLen-1);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);

		// update the indices used for rendering
		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
		glVertexPointer(3, GL_FLOAT, sizeof(VBOVertex), 0);
		glNormalPointer(GL_FLOAT, sizeof(VBOVertex), reinterpret_cast<void *>(3*sizeof(float)));
		glTexCoordPointer(2, GL_FLOAT, sizeof(VBOVertex), reinterpret_cast<void *>(6*sizeof(float)));
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, ctx->indices_vbo);
		glDrawElements(GL_TRIANGLES, ctx->indices_tri_count*3, GL_UNSIGNED_SHORT, 0);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glPopMatrix();
	}
};

GasGiant::GasGiant(const SystemBody *body) : BaseSphere(body), m_terrain(Terrain::InstanceTerrain(body)),
	m_hasTempCampos(false), m_tempCampos(0.0)
{
	//SetUpMaterials is not called until first Render since light count is zero :)

	BuildFirstPatches();
	GenerateTexture();
}

GasGiant::~GasGiant()
{
}

#define DUMP_TO_TEXTURE 0

#if DUMP_TO_TEXTURE
#include "FileSystem.h"
#include "PngWriter.h"
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

/* in patch surface coords, [0,1] */
vector3d GetSpherePoint(const double x, const double y, const vector3d *v) {
	return (v[0] + x*(1.0-y)*(v[1]-v[0]) + x*y*(v[2]-v[0]) + (1.0-x)*y*(v[3]-v[0])).Normalized();
}

static const Uint32 UV_DIMS = 512;
static const double FRACSTEP = 1.0 / double(UV_DIMS-1);

// generate root face patches of the cube/sphere
static const vector3d p1 = (vector3d( 1, 1, 1)).Normalized();
static const vector3d p2 = (vector3d(-1, 1, 1)).Normalized();
static const vector3d p3 = (vector3d(-1,-1, 1)).Normalized();
static const vector3d p4 = (vector3d( 1,-1, 1)).Normalized();
static const vector3d p5 = (vector3d( 1, 1,-1)).Normalized();
static const vector3d p6 = (vector3d(-1, 1,-1)).Normalized();
static const vector3d p7 = (vector3d(-1,-1,-1)).Normalized();
static const vector3d p8 = (vector3d( 1,-1,-1)).Normalized();

static const vector3d s_patchFaces[NUM_PATCHES][4] = 
{ 
	{p5, p1, p4, p8}, // +x
	{p2, p6, p7, p3}, // -x
	
	{p2, p1, p5, p6}, // +y
	{p7, p8, p4, p3}, // -y

	{p6, p5, p8, p7}, // +z - NB: these are actually reversed!
	{p1, p2, p3, p4}  // -z
};

#define USE_PATCH_COLOUR_MARKERS 0
#if USE_PATCH_COLOUR_MARKERS
static const Color4f s_patchColours[NUM_PATCHES] = 
{
	Color4f(1.0f, 0.0f, 0.0f, 1.0f), // +x
	Color4f(0.0f, 0.5f, 0.5f, 1.0f), // -x

	Color4f(0.0f, 1.0f, 0.0f, 1.0f), // +y
	Color4f(0.5f, 0.0f, 0.5f, 1.0f), // -y

	Color4f(0.0f, 0.0f, 1.0f, 1.0f), // +z
	Color4f(0.5f, 0.5f, 0.0f, 1.0f)  // -z
};
#endif

void GasGiant::GenerateTexture()
{
	std::unique_ptr<Color, FreeDeleter> buf[NUM_PATCHES];
	for(int i=0; i<NUM_PATCHES; i++) {
		buf[i].reset(static_cast<Color*>(malloc(UV_DIMS * UV_DIMS * 4)));
	}

	for(int i=0; i<NUM_PATCHES; i++) {
#if USE_PATCH_COLOUR_MARKERS
		const Color4f tempCol = s_patchColours[i];
#endif
		Color* const pBuf = buf[i].get();
		for( Uint32 v=0; v<UV_DIMS; v++ ) {
			for( Uint32 u=0; u<UV_DIMS; u++ ) {
				// where in this row & colum are we now.
				const double ustep = double(u) * FRACSTEP;
				const double vstep = double(v) * FRACSTEP;

				// get point on the surface of the sphere
				const vector3d p = GetSpherePoint(ustep, vstep, &s_patchFaces[i][0]);
#if 1
#if USE_PATCH_COLOUR_MARKERS
				const bool bSolidColour = (ustep > 0.4 && ustep < 0.6) && (vstep > 0.4 && vstep < 0.6);
				// get colour using `p`
				const vector3d colour = bSolidColour ? vector3d(tempCol.r, tempCol.g, tempCol.b) : m_terrain->GetColor(p, 0.0, p);
#else
				// get colour using `p`
				const vector3d colour = m_terrain->GetColor(p, 0.0, p);
#endif

				// convert to ubyte and store
				Color* col = pBuf + (u + (v * UV_DIMS));
				col[0].r = Uint8(colour.x * 255.0);
				col[0].g = Uint8(colour.y * 255.0);
				col[0].b = Uint8(colour.z * 255.0);
#else
				// convert to ubyte and store
				Color* col = pBuf + (u + (v * UV_DIMS));
				col[0].r = Uint8(((p1.x + p.x) / p1.x*2.0) * 255.0);
				col[0].g = Uint8(((p1.x + p.y) / p1.x*2.0) * 255.0);
				col[0].b = Uint8(((p1.x + p.z) / p1.x*2.0) * 255.0);
#endif
				col[0].a = 255;
			}
		}
	}

#if DUMP_TO_TEXTURE
	char filename[1024];
	for(int i=0; i<NUM_PATCHES; i++) {
		snprintf(filename, 1024, "%s%d.png", GetSystemBody()->name.c_str(), i);
		textureDump(filename, UV_DIMS, UV_DIMS, buf[i].get());
	}
#endif

	// create texture
	const vector2f texSize(1.0f, 1.0f);
	const vector2f dataSize(UV_DIMS, UV_DIMS);
	const Graphics::TextureDescriptor texDesc(
		Graphics::TEXTURE_RGBA_8888, 
		dataSize, texSize, Graphics::LINEAR_CLAMP, 
		false, false, 0, Graphics::TEXTURE_CUBE_MAP);
	m_surfaceTexture.Reset(Pi::renderer->CreateTexture(texDesc));

	// update with buffer from above
	Graphics::TextureCubeData tcd;
	tcd.posX = buf[0].get();
	tcd.negX = buf[1].get();
	tcd.posY = buf[2].get();
	tcd.negY = buf[3].get();
	tcd.posZ = buf[4].get();
	tcd.negZ = buf[5].get();
	m_surfaceTexture->Update(tcd, dataSize, Graphics::TEXTURE_RGBA_8888);
}

static const float g_ambient[4] = { 0, 0, 0, 1.0 };

static void DrawAtmosphereSurface(Graphics::Renderer *renderer,
	const matrix4x4d &modelView, const vector3d &campos, float rad, Graphics::Material *mat)
{
	const int LAT_SEGS = 20;
	const int LONG_SEGS = 20;
	vector3d yaxis = campos.Normalized();
	vector3d zaxis = vector3d(1.0,0.0,0.0).Cross(yaxis).Normalized();
	vector3d xaxis = yaxis.Cross(zaxis);
	const matrix4x4d invrot = matrix4x4d::MakeRotMatrix(xaxis, yaxis, zaxis).InverseOf();

	renderer->SetTransform(modelView * matrix4x4d::ScaleMatrix(rad, rad, rad) * invrot);

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
}

void GasGiant::Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const float scale, const std::vector<Camera::Shadow> &shadows)
{
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

			renderer->SetBlendMode(Graphics::BLEND_ALPHA_ONE);
			renderer->SetDepthWrite(false);
			// make atmosphere sphere slightly bigger than required so
			// that the edges of the pixel shader atmosphere jizz doesn't
			// show ugly polygonal angles
			DrawAtmosphereSurface(renderer, trans, campos, m_materialParameters.atmosphere.atmosRadius*1.01, m_atmosphereMaterial.get());
			renderer->SetDepthWrite(true);
			renderer->SetBlendMode(Graphics::BLEND_SOLID);
		}
	}

	Color ambient;
	Color &emission = m_surfaceMaterial->emissive;

	// save old global ambient
	const Color oldAmbient = renderer->GetAmbientColor();

	if ((GetSystemBody()->GetSuperType() == SystemBody::SUPERTYPE_STAR) || (GetSystemBody()->type == SystemBody::TYPE_BROWN_DWARF)) {
		// stars should emit light and terrain should be visible from distance
		ambient.r = ambient.g = ambient.b = 51;
		ambient.a = 255;
		emission.r = StarSystem::starRealColors[GetSystemBody()->type][0];
		emission.g = StarSystem::starRealColors[GetSystemBody()->type][1];
		emission.b = StarSystem::starRealColors[GetSystemBody()->type][2];
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
//#define USE_WIREFRAME
#ifdef USE_WIREFRAME
	renderer->SetWireFrameMode(true);
#endif
	// this is pretty much the only place where a non-renderer is allowed to call Apply()
	// to be removed when someone rewrites terrain
	m_surfaceMaterial->Apply();

	renderer->SetTransform(modelView);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	for (int i=0; i<NUM_PATCHES; i++) {
		m_patches[i]->Render(renderer, campos, modelView, frustum);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	m_surfaceMaterial->Unapply();

	renderer->SetAmbientColor(oldAmbient);
#ifdef USE_WIREFRAME
	renderer->SetWireFrameMode(false);
#endif
}

void GasGiant::SetUpMaterials()
{
	// Request material for this star or planet, with or without
	// atmosphere. Separate material for surface and sky.
	Graphics::MaterialDescriptor surfDesc;
	surfDesc.effect = Graphics::EFFECT_GASSPHERE_TERRAIN;

	if ((GetSystemBody()->type == SystemBody::TYPE_BROWN_DWARF) ||
		(GetSystemBody()->type == SystemBody::TYPE_STAR_M)) {
		//dim star (emits and receives light)
		surfDesc.lighting = true;
		surfDesc.quality &= ~Graphics::HAS_ATMOSPHERE;
	}
	else if (GetSystemBody()->GetSuperType() == SystemBody::SUPERTYPE_STAR) {
		//normal star
		surfDesc.lighting = false;
		surfDesc.quality &= ~Graphics::HAS_ATMOSPHERE;
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

	const bool bEnableEclipse = (Pi::config->Int("DisableEclipse") == 0);
	if (bEnableEclipse) {
		surfDesc.quality |= Graphics::HAS_ECLIPSES;
	}
	surfDesc.textures = 1;
	m_surfaceMaterial.Reset(Pi::renderer->CreateMaterial(surfDesc));
	m_surfaceMaterial->texture0 = m_surfaceTexture.Get();

	//Shader-less atmosphere is drawn in Planet
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
	if( s_patchContext.Get() == nullptr ) {
		s_patchContext.Reset(new GasPatchContext(127));
	}

	// generate root face patches of the cube/sphere
	static const vector3d p1 = (vector3d( 1, 1, 1)).Normalized();
	static const vector3d p2 = (vector3d(-1, 1, 1)).Normalized();
	static const vector3d p3 = (vector3d(-1,-1, 1)).Normalized();
	static const vector3d p4 = (vector3d( 1,-1, 1)).Normalized();
	static const vector3d p5 = (vector3d( 1, 1,-1)).Normalized();
	static const vector3d p6 = (vector3d(-1, 1,-1)).Normalized();
	static const vector3d p7 = (vector3d(-1,-1,-1)).Normalized();
	static const vector3d p8 = (vector3d( 1,-1,-1)).Normalized();

	m_patches[0].reset(new GasPatch(s_patchContext, this, p1, p2, p3, p4));
	m_patches[1].reset(new GasPatch(s_patchContext, this, p4, p3, p7, p8));
	m_patches[2].reset(new GasPatch(s_patchContext, this, p1, p4, p8, p5));
	m_patches[3].reset(new GasPatch(s_patchContext, this, p2, p1, p5, p6));
	m_patches[4].reset(new GasPatch(s_patchContext, this, p3, p2, p6, p7));
	m_patches[5].reset(new GasPatch(s_patchContext, this, p8, p7, p6, p5));
}
