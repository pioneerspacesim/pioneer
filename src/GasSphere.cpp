// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "GasSphere.h"
#include "perlin.h"
#include "Pi.h"
#include "RefCounted.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/VertexArray.h"
#include "vcacheopt/vcacheopt.h"
#include <deque>
#include <algorithm>


RefCountedPtr<GasPatchContext> GasSphere::s_patchContext;

static void print_info(const SystemBody *sbody, const Terrain *terrain)
{
	printf(
		"%s:\n"
		"    height fractal: %s\n"
		"    colour fractal: %s\n"
		"    seed: %u\n",
		sbody->GetName().c_str(), terrain->GetHeightFractalName(), terrain->GetColorFractalName(), sbody->GetSeed());
}

#pragma pack(4)
struct VBOVertex
{
	float x,y,z;
	float nx,ny,nz;
	unsigned char col[4];
	float padding;
};
#pragma pack()

static const int INDEX_LISTS = 16;

class GasPatchContext : public RefCounted {
public:
	int edgeLen;

	inline int VBO_COUNT_LO_EDGE() const { return 3*(edgeLen/2); }
	inline int VBO_COUNT_HI_EDGE() const { return 3*(edgeLen-1); }
	inline int VBO_COUNT_MID_IDX() const { return (4*3*(edgeLen-3))    + 2*(edgeLen-3)*(edgeLen-3)*3; }
	//                                            ^^ serrated teeth bit  ^^^ square inner bit

	inline int IDX_VBO_LO_OFFSET(int i) const { return i*sizeof(unsigned short)*3*(edgeLen/2); }
	inline int IDX_VBO_HI_OFFSET(int i) const { return (i*sizeof(unsigned short)*VBO_COUNT_HI_EDGE())+IDX_VBO_LO_OFFSET(4); }
	inline int IDX_VBO_MAIN_OFFSET()    const { return IDX_VBO_HI_OFFSET(4); }
	inline int IDX_VBO_COUNT_ALL_IDX()	const { return ((edgeLen-1)*(edgeLen-1))*2*3; }

	inline int NUMVERTICES() const { return edgeLen*edgeLen; }

	double frac;

	std::unique_ptr<unsigned short[]> midIndices;
	std::unique_ptr<unsigned short[]> loEdgeIndices[4];
	std::unique_ptr<unsigned short[]> hiEdgeIndices[4];
	GLuint indices_vbo;
	GLuint indices_list[INDEX_LISTS];
	GLuint indices_tri_count;
	GLuint indices_tri_counts[INDEX_LISTS];
	VBOVertex *vbotemp;

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
		midIndices.reset();
		for (int i=0; i<4; i++) {
			loEdgeIndices[i].reset();
			hiEdgeIndices[i].reset();
		}
		if (indices_vbo) {
			indices_vbo = 0;
		}
		for (int i=0; i<INDEX_LISTS; i++) {
			if (indices_list[i]) {
				glDeleteBuffersARB(1, &indices_list[i]);
			}
		}
		delete [] vbotemp;
	}

	void updateIndexBufferId(const GLuint edge_hi_flags) {
		assert(edge_hi_flags < GLuint(INDEX_LISTS));
		indices_vbo = indices_list[edge_hi_flags];
		indices_tri_count = indices_tri_counts[edge_hi_flags];
	}

	int getIndices(std::vector<unsigned short> &pl, const unsigned int edge_hi_flags)
	{
		// calculate how many tri's there are
		int tri_count = (VBO_COUNT_MID_IDX() / 3);
		for( int i=0; i<4; ++i ) {
			if( edge_hi_flags & (1 << i) ) {
				tri_count += (VBO_COUNT_HI_EDGE() / 3);
			} else {
				tri_count += (VBO_COUNT_LO_EDGE() / 3);
			}
		}

		// pre-allocate enough space
		pl.reserve(tri_count);

		// add all of the middle indices
		for(int i=0; i<VBO_COUNT_MID_IDX(); ++i) {
			pl.push_back(midIndices[i]);
		}
		// selectively add the HI or LO detail indices
		for (int i=0; i<4; i++) {
			if( edge_hi_flags & (1 << i) ) {
				for(int j=0; j<VBO_COUNT_HI_EDGE(); ++j) {
					pl.push_back(hiEdgeIndices[i][j]);
				}
			} else {
				for(int j=0; j<VBO_COUNT_LO_EDGE(); ++j) {
					pl.push_back(loEdgeIndices[i][j]);
				}
			}
		}

		return tri_count;
	}

	void Init() {
		frac = 1.0 / double(edgeLen-1);

		vbotemp = new VBOVertex[NUMVERTICES()];

		unsigned short *idx;
		midIndices.reset(new unsigned short[VBO_COUNT_MID_IDX()]);
		for (int i=0; i<4; i++) {
			loEdgeIndices[i].reset(new unsigned short[VBO_COUNT_LO_EDGE()]);
			hiEdgeIndices[i].reset(new unsigned short[VBO_COUNT_HI_EDGE()]);
		}
		/* also want vtx indices for tris not touching edge of patch */
		idx = midIndices.get();
		for (int x=1; x<edgeLen-2; x++) {
			for (int y=1; y<edgeLen-2; y++) {
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
		{
			for (int x=1; x<edgeLen-3; x+=2) {
				// razor teeth near edge 0
				idx[0] = x + edgeLen;
				idx[1] = x+1;
				idx[2] = x+1 + edgeLen;
				idx+=3;
				idx[0] = x+1;
				idx[1] = x+2 + edgeLen;
				idx[2] = x+1 + edgeLen;
				idx+=3;
			}
			for (int x=1; x<edgeLen-3; x+=2) {
				// near edge 2
				idx[0] = x + edgeLen*(edgeLen-2);
				idx[1] = x+1 + edgeLen*(edgeLen-2);
				idx[2] = x+1 + edgeLen*(edgeLen-1);
				idx+=3;
				idx[0] = x+1 + edgeLen*(edgeLen-2);
				idx[1] = x+2 + edgeLen*(edgeLen-2);
				idx[2] = x+1 + edgeLen*(edgeLen-1);
				idx+=3;
			}
			for (int y=1; y<edgeLen-3; y+=2) {
				// near edge 1
				idx[0] = edgeLen-2 + y*edgeLen;
				idx[1] = edgeLen-1 + (y+1)*edgeLen;
				idx[2] = edgeLen-2 + (y+1)*edgeLen;
				idx+=3;
				idx[0] = edgeLen-2 + (y+1)*edgeLen;
				idx[1] = edgeLen-1 + (y+1)*edgeLen;
				idx[2] = edgeLen-2 + (y+2)*edgeLen;
				idx+=3;
			}
			for (int y=1; y<edgeLen-3; y+=2) {
				// near edge 3
				idx[0] = 1 + y*edgeLen;
				idx[1] = 1 + (y+1)*edgeLen;
				idx[2] = (y+1)*edgeLen;
				idx+=3;
				idx[0] = 1 + (y+1)*edgeLen;
				idx[1] = 1 + (y+2)*edgeLen;
				idx[2] = (y+1)*edgeLen;
				idx+=3;
			}
		}
		// full detail edge triangles
		{
			idx = hiEdgeIndices[0].get();
			for (int x=0; x<edgeLen-1; x+=2) {
				idx[0] = x; idx[1] = x+1; idx[2] = x+1 + edgeLen;
				idx+=3;
				idx[0] = x+1; idx[1] = x+2; idx[2] = x+1 + edgeLen;
				idx+=3;
			}
			idx = hiEdgeIndices[1].get();
			for (int y=0; y<edgeLen-1; y+=2) {
				idx[0] = edgeLen-1 + y*edgeLen;
				idx[1] = edgeLen-1 + (y+1)*edgeLen;
				idx[2] = edgeLen-2 + (y+1)*edgeLen;
				idx+=3;
				idx[0] = edgeLen-1 + (y+1)*edgeLen;
				idx[1] = edgeLen-1 + (y+2)*edgeLen;
				idx[2] = edgeLen-2 + (y+1)*edgeLen;
				idx+=3;
			}
			idx = hiEdgeIndices[2].get();
			for (int x=0; x<edgeLen-1; x+=2) {
				idx[0] = x + (edgeLen-1)*edgeLen;
				idx[1] = x+1 + (edgeLen-2)*edgeLen;
				idx[2] = x+1 + (edgeLen-1)*edgeLen;
				idx+=3;
				idx[0] = x+1 + (edgeLen-2)*edgeLen;
				idx[1] = x+2 + (edgeLen-1)*edgeLen;
				idx[2] = x+1 + (edgeLen-1)*edgeLen;
				idx+=3;
			}
			idx = hiEdgeIndices[3].get();
			for (int y=0; y<edgeLen-1; y+=2) {
				idx[0] = y*edgeLen;
				idx[1] = 1 + (y+1)*edgeLen;
				idx[2] = (y+1)*edgeLen;
				idx+=3;
				idx[0] = (y+1)*edgeLen;
				idx[1] = 1 + (y+1)*edgeLen;
				idx[2] = (y+2)*edgeLen;
				idx+=3;
			}
		}
		// these edge indices are for patches with no
		// neighbour of equal or greater detail -- they reduce
		// their edge complexity by 1 division
		{
			idx = loEdgeIndices[0].get();
			for (int x=0; x<edgeLen-2; x+=2) {
				idx[0] = x;
				idx[1] = x+2;
				idx[2] = x+1+edgeLen;
				idx += 3;
			}
			idx = loEdgeIndices[1].get();
			for (int y=0; y<edgeLen-2; y+=2) {
				idx[0] = (edgeLen-1) + y*edgeLen;
				idx[1] = (edgeLen-1) + (y+2)*edgeLen;
				idx[2] = (edgeLen-2) + (y+1)*edgeLen;
				idx += 3;
			}
			idx = loEdgeIndices[2].get();
			for (int x=0; x<edgeLen-2; x+=2) {
				idx[0] = x+edgeLen*(edgeLen-1);
				idx[2] = x+2+edgeLen*(edgeLen-1);
				idx[1] = x+1+edgeLen*(edgeLen-2);
				idx += 3;
			}
			idx = loEdgeIndices[3].get();
			for (int y=0; y<edgeLen-2; y+=2) {
				idx[0] = y*edgeLen;
				idx[2] = (y+2)*edgeLen;
				idx[1] = 1 + (y+1)*edgeLen;
				idx += 3;
			}
		}

		// these will hold the optimised indices
		std::vector<unsigned short> pl_short[INDEX_LISTS];
		// populate the N indices lists from the arrays built during InitTerrainIndices()
		for( int i=0; i<INDEX_LISTS; ++i ) {
			const unsigned int edge_hi_flags = i;
			indices_tri_counts[i] = getIndices(pl_short[i], edge_hi_flags);
		}

		// iterate over each index list and optimize it
		for( int i=0; i<INDEX_LISTS; ++i ) {
			int tri_count = indices_tri_counts[i];
			VertexCacheOptimizerUShort vco;
			VertexCacheOptimizerUShort::Result res = vco.Optimize(&pl_short[i][0], tri_count);
			assert(0 == res);
		}

		// everything should be hunky-dory for setting up as OpenGL index buffers now.
		for( int i=0; i<INDEX_LISTS; ++i ) {
			glGenBuffersARB(1, &indices_list[i]);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_list[i]);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*indices_tri_counts[i]*3, &(pl_short[i][0]), GL_STATIC_DRAW);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
		}

		// default it to the last entry which uses the hi-res borders
		indices_vbo			= indices_list[INDEX_LISTS-1];
		indices_tri_count	= indices_tri_counts[INDEX_LISTS-1];

		if (midIndices) {
			midIndices.reset();
			for (int i=0; i<4; i++) {
				loEdgeIndices[i].reset();
				hiEdgeIndices[i].reset();
			}
		}
	}
};


class GasPatch {
public:
	RefCountedPtr<GasPatchContext> ctx;
	vector3d v[4];
	vector3d *vertices;
	vector3d *normals;
	vector3d *colors;
	GLuint m_vbo;
	GasSphere *gasSphere;
	double m_roughLength;
	vector3d clipCentroid, centroid;
	double clipRadius;

	GasPatch(const RefCountedPtr<GasPatchContext> &_ctx, GasSphere *gs, vector3d v0, vector3d v1, vector3d v2, vector3d v3) {
		memset(this, 0, sizeof(GasPatch));

		ctx = _ctx;

		gasSphere = gs;

		v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3;
		clipCentroid = (v0+v1+v2+v3) * 0.25;
		clipRadius = 0;
		for (int i=0; i<4; i++) {
			clipRadius = std::max(clipRadius, (v[i]-clipCentroid).Length());
		}
		normals = new vector3d[ctx->NUMVERTICES()];
		vertices = new vector3d[ctx->NUMVERTICES()];
		colors = new vector3d[ctx->NUMVERTICES()];

		GenerateMesh();
		UpdateVBOs();
	}

	~GasPatch() {
		delete[] vertices;
		delete[] normals;
		delete[] colors;
		glDeleteBuffersARB(1, &m_vbo);
	}

	void UpdateVBOs() {
		if (!m_vbo) 
			glGenBuffersARB(1, &m_vbo);
		
		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(VBOVertex)*ctx->NUMVERTICES(), 0, GL_DYNAMIC_DRAW);
		for (int i=0; i<ctx->NUMVERTICES(); i++)
		{
			clipRadius = std::max(clipRadius, (vertices[i]-clipCentroid).Length());
			VBOVertex *pData = ctx->vbotemp + i;
			pData->x = float(vertices[i].x - clipCentroid.x);
			pData->y = float(vertices[i].y - clipCentroid.y);
			pData->z = float(vertices[i].z - clipCentroid.z);
			pData->nx = float(normals[i].x);
			pData->ny = float(normals[i].y);
			pData->nz = float(normals[i].z);
			pData->col[0] = static_cast<unsigned char>(Clamp(colors[i].x*255.0, 0.0, 255.0));
			pData->col[1] = static_cast<unsigned char>(Clamp(colors[i].y*255.0, 0.0, 255.0));
			pData->col[2] = static_cast<unsigned char>(Clamp(colors[i].z*255.0, 0.0, 255.0));
			pData->col[3] = 255;
		}
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(VBOVertex)*ctx->NUMVERTICES(), ctx->vbotemp, GL_DYNAMIC_DRAW);
		glBindBufferARB(GL_ARRAY_BUFFER, 0);
	}

	/* in patch surface coords, [0,1] */
	vector3d GetSpherePoint(const double x, const double y) const {
		return (v[0] + x*(1.0-y)*(v[1]-v[0]) +
			    x*y*(v[2]-v[0]) +
			    (1.0-x)*y*(v[3]-v[0])).Normalized();
	}

	/** Generates full-detail vertices, and also non-edge normals and colors */
	void GenerateMesh() {
		centroid = clipCentroid.Normalized();
		centroid = (1.0 + gasSphere->GetHeight(centroid)) * centroid;
		vector3d *vts = vertices;
		vector3d *nrm = normals;
		vector3d *col = colors;
		for (int y=0; y<ctx->edgeLen; y++) {
			for (int x=0; x<ctx->edgeLen; x++) {
				// get the position on the surface of the sphere
				const vector3d p = GetSpherePoint(x*ctx->frac, y*ctx->frac);
				// store it
				*(vts++) = p;
				*(nrm++) = p;
				*(col++) = vector3d();
			}
		}
	}

	GLuint determineIndexbuffer() const {
		return 1u | 2u | 4u | 8u;
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
		glEnableClientState(GL_COLOR_ARRAY);

		// update the indices used for rendering
		ctx->updateIndexBufferId(determineIndexbuffer());

		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
		glVertexPointer(3, GL_FLOAT, sizeof(VBOVertex), 0);
		glNormalPointer(GL_FLOAT, sizeof(VBOVertex), reinterpret_cast<void *>(3*sizeof(float)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VBOVertex), reinterpret_cast<void *>(6*sizeof(float)));
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, ctx->indices_vbo);
		glDrawElements(GL_TRIANGLES, ctx->indices_tri_count*3, GL_UNSIGNED_SHORT, 0);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glPopMatrix();
	}
};

GasSphere::GasSphere(const SystemBody *body) : BaseSphere(body),
	m_hasTempCampos(false), m_tempCampos(0.0)
{
	//SetUpMaterials is not called until first Render since light count is zero :)

	BuildFirstPatches();
}

GasSphere::~GasSphere()
{
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

void GasSphere::Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const float scale, const std::vector<Camera::Shadow> &shadows)
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
	glEnableClientState(GL_COLOR_ARRAY);

	for (int i=0; i<NUM_PATCHES; i++) {
		m_patches[i]->Render(renderer, campos, modelView, frustum);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	m_surfaceMaterial->Unapply();

	renderer->SetAmbientColor(oldAmbient);
#ifdef USE_WIREFRAME
	renderer->SetWireFrameMode(false);
#endif
}

void GasSphere::SetUpMaterials()
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
	m_surfaceMaterial.Reset(Pi::renderer->CreateMaterial(surfDesc));

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

void GasSphere::BuildFirstPatches()
{
	if( s_patchContext.Get() == nullptr ) {
		s_patchContext.Reset(new GasPatchContext(55));
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
