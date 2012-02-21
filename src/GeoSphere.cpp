#include "libs.h"
#include "GeoSphere.h"
#include "perlin.h"
#include "Pi.h"
#include "StarSystem.h"
#include "RefCounted.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/Frustum.h"
#include "graphics/Graphics.h"
#include "graphics/VertexArray.h"
#include "graphics/Shader.h"
#include <deque>
#include <algorithm>

using namespace Graphics;

// tri edge lengths
#define GEOPATCH_SUBDIVIDE_AT_CAMDIST	5.0
#define GEOPATCH_MAX_DEPTH  15 + (2*Pi::detail.fracmult) //15 
#define GEOSPHERE_USE_THREADING

static const int GEOPATCH_MAX_EDGELEN = 55;
int GeoSphere::s_vtxGenCount = 0;
RefCountedPtr<GeoPatchContext> GeoSphere::s_patchContext;

// must be odd numbers
static const int detail_edgeLen[5] = {
	7, 15, 25, 35, 55
};


#define PRINT_VECTOR(_v) printf("%f,%f,%f\n", (_v).x, (_v).y, (_v).z);

SHADER_CLASS_BEGIN(GeosphereShader)
	SHADER_UNIFORM_VEC4(atmosColor)
	SHADER_UNIFORM_FLOAT(geosphereScale)
	SHADER_UNIFORM_FLOAT(geosphereAtmosTopRad)
	SHADER_UNIFORM_VEC3(geosphereCenter)
	SHADER_UNIFORM_FLOAT(geosphereAtmosFogDensity)
SHADER_CLASS_END()

static GeosphereShader *s_geosphereSurfaceShader[4], *s_geosphereSkyShader[4], *s_geosphereStarShader, *s_geosphereDimStarShader[4];

#pragma pack(4)
struct VBOVertex
{
	float x,y,z;
	float nx,ny,nz;
	unsigned char col[4];
	float padding;
};
#pragma pack()

// for glDrawRangeElements
static int s_loMinIdx[4], s_loMaxIdx[4];
static int s_hiMinIdx[4], s_hiMaxIdx[4];

class GeoPatchContext : public RefCounted {
public:
	int edgeLen;

	inline int VBO_COUNT_LO_EDGE() const { return 3*(edgeLen/2); }
	inline int VBO_COUNT_HI_EDGE() const { return 3*(edgeLen-1); }
	inline int VBO_COUNT_MID_IDX() const { return (4*3*(edgeLen-3))    + 2*(edgeLen-3)*(edgeLen-3)*3; }
	//                                            ^^ serrated teeth bit  ^^^ square inner bit

	inline int IDX_VBO_LO_OFFSET(int i) const { return i*sizeof(unsigned short)*3*(edgeLen/2); }
	inline int IDX_VBO_HI_OFFSET(int i) const { return (i*sizeof(unsigned short)*VBO_COUNT_HI_EDGE())+IDX_VBO_LO_OFFSET(4); }
	inline int IDX_VBO_MAIN_OFFSET()    const { return IDX_VBO_HI_OFFSET(4); }

	inline int NUMVERTICES() const { return edgeLen*edgeLen; }

	double frac;

	unsigned short *midIndices;
	unsigned short *loEdgeIndices[4];
	unsigned short *hiEdgeIndices[4];
	GLuint indices_vbo;
	VBOVertex *vbotemp;

	GeoPatchContext(int _edgeLen) : edgeLen(_edgeLen) {
		Init();
	}

	~GeoPatchContext() {
		Cleanup();
	}

	void Refresh() {
		Cleanup();
		Init();
	}

	void Cleanup() {
		delete [] midIndices;
		for (int i=0; i<4; i++) {
			delete [] loEdgeIndices[i];
			delete [] hiEdgeIndices[i];
		}
		if (indices_vbo) {
			glDeleteBuffersARB(1, &indices_vbo);
		}
		delete [] vbotemp;
	}

	void Init() {
		frac = 1.0 / double(edgeLen-1);

		vbotemp = new VBOVertex[NUMVERTICES()];
			
		unsigned short *idx;
		midIndices = new unsigned short[VBO_COUNT_MID_IDX()];
		for (int i=0; i<4; i++) {
			loEdgeIndices[i] = new unsigned short[VBO_COUNT_LO_EDGE()];
			hiEdgeIndices[i] = new unsigned short[VBO_COUNT_HI_EDGE()];
		}
		/* also want vtx indices for tris not touching edge of patch */
		idx = midIndices;
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
			idx = hiEdgeIndices[0];
			for (int x=0; x<edgeLen-1; x+=2) {
				idx[0] = x; idx[1] = x+1; idx[2] = x+1 + edgeLen;
				idx+=3;
				idx[0] = x+1; idx[1] = x+2; idx[2] = x+1 + edgeLen;
				idx+=3;
			}
			idx = hiEdgeIndices[1];
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
			idx = hiEdgeIndices[2];
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
			idx = hiEdgeIndices[3];
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
			idx = loEdgeIndices[0];
			for (int x=0; x<edgeLen-2; x+=2) {
				idx[0] = x;
				idx[1] = x+2;
				idx[2] = x+1+edgeLen;
				idx += 3;
			}
			idx = loEdgeIndices[1];
			for (int y=0; y<edgeLen-2; y+=2) {
				idx[0] = (edgeLen-1) + y*edgeLen;
				idx[1] = (edgeLen-1) + (y+2)*edgeLen;
				idx[2] = (edgeLen-2) + (y+1)*edgeLen;
				idx += 3;
			}
			idx = loEdgeIndices[2];
			for (int x=0; x<edgeLen-2; x+=2) {
				idx[0] = x+edgeLen*(edgeLen-1);
				idx[2] = x+2+edgeLen*(edgeLen-1);
				idx[1] = x+1+edgeLen*(edgeLen-2);
				idx += 3;
			}
			idx = loEdgeIndices[3];
			for (int y=0; y<edgeLen-2; y+=2) {
				idx[0] = y*edgeLen;
				idx[2] = (y+2)*edgeLen;
				idx[1] = 1 + (y+1)*edgeLen;
				idx += 3;
			}
		}
		// find min/max indices
		for (int i=0; i<4; i++) {
			s_loMinIdx[i] = s_hiMinIdx[i] = 1<<30;
			s_loMaxIdx[i] = s_hiMaxIdx[i] = 0;
			for (int j=0; j<3*(edgeLen/2); j++) {
				if (loEdgeIndices[i][j] < s_loMinIdx[i]) s_loMinIdx[i] = loEdgeIndices[i][j];
				if (loEdgeIndices[i][j] > s_loMaxIdx[i]) s_loMaxIdx[i] = loEdgeIndices[i][j];
			}
			for (int j=0; j<VBO_COUNT_HI_EDGE(); j++) {
				if (hiEdgeIndices[i][j] < s_hiMinIdx[i]) s_hiMinIdx[i] = hiEdgeIndices[i][j];
				if (hiEdgeIndices[i][j] > s_hiMaxIdx[i]) s_hiMaxIdx[i] = hiEdgeIndices[i][j];
			}
			//printf("%d:\nLo %d:%d\nHi: %d:%d\n", i, s_loMinIdx[i], s_loMaxIdx[i], s_hiMinIdx[i], s_hiMaxIdx[i]);
		}

		glGenBuffersARB(1, &indices_vbo);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, IDX_VBO_MAIN_OFFSET() + sizeof(unsigned short)*VBO_COUNT_MID_IDX(), 0, GL_STATIC_DRAW);
		for (int i=0; i<4; i++) {
			glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER, 
				IDX_VBO_LO_OFFSET(i),
				sizeof(unsigned short)*3*(edgeLen/2),
				loEdgeIndices[i]);
		}
		for (int i=0; i<4; i++) {
			glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER,
				IDX_VBO_HI_OFFSET(i),
				sizeof(unsigned short)*VBO_COUNT_HI_EDGE(),
				hiEdgeIndices[i]);
		}
		glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER,
				IDX_VBO_MAIN_OFFSET(),
				sizeof(unsigned short)*VBO_COUNT_MID_IDX(),
				midIndices);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void GetEdge(vector3d *array, int edge, vector3d *ev) {
		if (edge == 0) {
			for (int x=0; x<edgeLen; x++) ev[x] = array[x];
		} else if (edge == 1) {
			const int x = edgeLen-1;
			for (int y=0; y<edgeLen; y++) ev[y] = array[x + y*edgeLen];
		} else if (edge == 2) {
			const int y = edgeLen-1;
			for (int x=0; x<edgeLen; x++) ev[x] = array[(edgeLen-1)-x + y*edgeLen];
		} else {
			for (int y=0; y<edgeLen; y++) ev[y] = array[0 + ((edgeLen-1)-y)*edgeLen];
		}
	}

	void SetEdge(vector3d *array, int edge, const vector3d *ev) {
		if (edge == 0) {
			for (int x=0; x<edgeLen; x++) array[x] = ev[x];
		} else if (edge == 1) {
			const int x = edgeLen-1;
			for (int y=0; y<edgeLen; y++) array[x + y*edgeLen] = ev[y];
		} else if (edge == 2) {
			const int y = edgeLen-1;
			for (int x=0; x<edgeLen; x++) array[(edgeLen-1)-x + y*edgeLen] = ev[x];
		} else {
			for (int y=0; y<edgeLen; y++) array[0 + ((edgeLen-1)-y)*edgeLen] = ev[y];
		}
	}
};


class GeoPatch {
public:
	RefCountedPtr<GeoPatchContext> ctx;
	vector3d v[4];
	vector3d *vertices;
	vector3d *normals;
	vector3d *colors;
	GLuint m_vbo;
	GeoPatch *kids[4];
	GeoPatch *parent;
	GeoPatch *edgeFriend[4]; // [0]=v01, [1]=v12, [2]=v20
	GeoSphere *geosphere;
	double m_roughLength;
	vector3d clipCentroid, centroid;
	double clipRadius;
	int m_depth;
	SDL_mutex *m_kidsLock;
	bool m_needUpdateVBOs;
	double m_distMult;
	
	GeoPatch(const RefCountedPtr<GeoPatchContext> &_ctx, GeoSphere *gs, vector3d v0, vector3d v1, vector3d v2, vector3d v3, int depth) {
		memset(this, 0, sizeof(GeoPatch));

		ctx = _ctx;

		geosphere = gs;

		m_kidsLock = SDL_CreateMutex();
		v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3;
		//depth -= Pi::detail.fracmult;
		m_depth = depth;
		clipCentroid = (v0+v1+v2+v3) * 0.25;
		clipRadius = 0;
		for (int i=0; i<4; i++) {
			clipRadius = std::max(clipRadius, (v[i]-clipCentroid).Length());
		}
		if (geosphere->m_sbody->type < SBody::TYPE_PLANET_ASTEROID) {
 			m_distMult = 10 / Clamp(depth, 1, 10);
 		} else {
 			m_distMult = 5 / Clamp(depth, 1, 5);
 		}
		m_roughLength = GEOPATCH_SUBDIVIDE_AT_CAMDIST / pow(2.0, depth) * m_distMult;
		m_needUpdateVBOs = false;
		normals = new vector3d[ctx->NUMVERTICES()];
		vertices = new vector3d[ctx->NUMVERTICES()];
		colors = new vector3d[ctx->NUMVERTICES()];
	}

	~GeoPatch() {
		SDL_DestroyMutex(m_kidsLock);
		for (int i=0; i<4; i++) {
			if (edgeFriend[i]) edgeFriend[i]->NotifyEdgeFriendDeleted(this);
		}
		for (int i=0; i<4; i++) if (kids[i]) delete kids[i];
		delete[] vertices;
		delete[] normals;
		delete[] colors;
		geosphere->AddVBOToDestroy(m_vbo);
	}

	void UpdateVBOs() {
		m_needUpdateVBOs = true;
	}

	void _UpdateVBOs() {
		if (m_needUpdateVBOs) {
			if (!m_vbo) glGenBuffersARB(1, &m_vbo);
			m_needUpdateVBOs = false;
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
	}	
	/* not quite edge, since we share edge vertices so that would be
	 * fucking pointless. one position inwards. used to make edge normals
	 * for adjacent tiles */
	void GetEdgeMinusOneVerticesFlipped(int edge, vector3d *ev) {
		if (edge == 0) {
			for (int x=0; x<ctx->edgeLen; x++) ev[ctx->edgeLen-1-x] = vertices[x + ctx->edgeLen];
		} else if (edge == 1) {
			const int x = ctx->edgeLen-2;
			for (int y=0; y<ctx->edgeLen; y++) ev[ctx->edgeLen-1-y] = vertices[x + y*ctx->edgeLen];
		} else if (edge == 2) {
			const int y = ctx->edgeLen-2;
			for (int x=0; x<ctx->edgeLen; x++) ev[ctx->edgeLen-1-x] = vertices[(ctx->edgeLen-1)-x + y*ctx->edgeLen];
		} else {
			for (int y=0; y<ctx->edgeLen; y++) ev[ctx->edgeLen-1-y] = vertices[1 + ((ctx->edgeLen-1)-y)*ctx->edgeLen];
		}
	}
	int GetEdgeIdxOf(GeoPatch *e) {
		for (int i=0; i<4; i++) {
			if (edgeFriend[i] == e) return i;
		}
		abort();
		return -1;
	}


	void FixEdgeNormals(const int edge, const vector3d *ev) {
		int x, y;
		switch (edge) {
		case 0:
			for (x=1; x<ctx->edgeLen-1; x++) {
				const vector3d x1 = vertices[x-1];
				const vector3d x2 = vertices[x+1];
				const vector3d y1 = ev[x];
				const vector3d y2 = vertices[x + ctx->edgeLen];
				const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
				normals[x] = norm;
				// make color
				const vector3d p = GetSpherePoint(x*ctx->frac, 0);
				const double height = colors[x].x;
				colors[x] = geosphere->GetColor(p, height, norm);
			}
			break;
		case 1:
			x = ctx->edgeLen-1;
			for (y=1; y<ctx->edgeLen-1; y++) {
				const vector3d x1 = vertices[(x-1) + y*ctx->edgeLen];
				const vector3d x2 = ev[y];
				const vector3d y1 = vertices[x + (y-1)*ctx->edgeLen];
				const vector3d y2 = vertices[x + (y+1)*ctx->edgeLen];
				const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
				normals[x + y*ctx->edgeLen] = norm;
				// make color
				const vector3d p = GetSpherePoint(x*ctx->frac, y*ctx->frac);
				const double height = colors[x + y*ctx->edgeLen].x;
				colors[x + y*ctx->edgeLen] = geosphere->GetColor(p, height, norm);
	//			colors[x+y*ctx->edgeLen] = vector3d(1,0,0);
			}
			break;
		case 2:
			y = ctx->edgeLen-1;
			for (x=1; x<ctx->edgeLen-1; x++) {
				const vector3d x1 = vertices[x-1 + y*ctx->edgeLen];
				const vector3d x2 = vertices[x+1 + y*ctx->edgeLen];
				const vector3d y1 = vertices[x + (y-1)*ctx->edgeLen];
				const vector3d y2 = ev[ctx->edgeLen-1-x];
				const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
				normals[x + y*ctx->edgeLen] = norm;
				// make color
				const vector3d p = GetSpherePoint(x*ctx->frac, y*ctx->frac);
				const double height = colors[x + y*ctx->edgeLen].x;
				colors[x + y*ctx->edgeLen] = geosphere->GetColor(p, height, norm);
			}
			break;
		case 3:
			for (y=1; y<ctx->edgeLen-1; y++) {
				const vector3d x1 = ev[ctx->edgeLen-1-y];
				const vector3d x2 = vertices[1 + y*ctx->edgeLen];
				const vector3d y1 = vertices[(y-1)*ctx->edgeLen];
				const vector3d y2 = vertices[(y+1)*ctx->edgeLen];
				const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
				normals[y*ctx->edgeLen] = norm;
				// make color
				const vector3d p = GetSpherePoint(0, y*ctx->frac);
				const double height = colors[y*ctx->edgeLen].x;
				colors[y*ctx->edgeLen] = geosphere->GetColor(p, height, norm);
	//			colors[y*ctx->edgeLen] = vector3d(0,1,0);
			}
			break;
		}
	}

	int GetChildIdx(GeoPatch *child) {
		for (int i=0; i<4; i++) {
			if (kids[i] == child) return i;
		}
		abort();
		return -1;
	}
	
	void FixEdgeFromParentInterpolated(int edge) {
		// noticeable artefacts from not doing so...
		vector3d ev[GEOPATCH_MAX_EDGELEN];
		vector3d en[GEOPATCH_MAX_EDGELEN];
		vector3d ec[GEOPATCH_MAX_EDGELEN];
		vector3d ev2[GEOPATCH_MAX_EDGELEN];
		vector3d en2[GEOPATCH_MAX_EDGELEN];
		vector3d ec2[GEOPATCH_MAX_EDGELEN];
		ctx->GetEdge(parent->vertices, edge, ev);
		ctx->GetEdge(parent->normals, edge, en);
		ctx->GetEdge(parent->colors, edge, ec);

		int kid_idx = parent->GetChildIdx(this);
		if (edge == kid_idx) {
			// use first half of edge
			for (int i=0; i<=ctx->edgeLen/2; i++) {
				ev2[i<<1] = ev[i];
				en2[i<<1] = en[i];
				ec2[i<<1] = ec[i];
			}
		} else {
			// use 2nd half of edge
			for (int i=ctx->edgeLen/2; i<ctx->edgeLen; i++) {
				ev2[(i-(ctx->edgeLen/2))<<1] = ev[i];
				en2[(i-(ctx->edgeLen/2))<<1] = en[i];
				ec2[(i-(ctx->edgeLen/2))<<1] = ec[i];
			}
		}
		// interpolate!!
		for (int i=1; i<ctx->edgeLen; i+=2) {
			ev2[i] = (ev2[i-1]+ev2[i+1]) * 0.5;
			en2[i] = (en2[i-1]+en2[i+1]).Normalized();
			ec2[i] = (ec2[i-1]+ec2[i+1]) * 0.5;
		}
		ctx->SetEdge(this->vertices, edge, ev2);
		ctx->SetEdge(this->normals, edge, en2);
		ctx->SetEdge(this->colors, edge, ec2);
	}

	template <int corner>
	void MakeCornerNormal(vector3d *ev, vector3d *ev2) {
		int p;
		vector3d x1,x2,y1,y2;
		switch (corner) {
		case 0: {
			x1 = ev[ctx->edgeLen-1];
			x2 = vertices[1];
			y1 = ev2[0];
			y2 = vertices[ctx->edgeLen];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[0] = norm;
			// make color
			const vector3d pt = GetSpherePoint(0, 0);
		//	const double height = colors[0].x;
			const double height = geosphere->GetHeight(pt);
			colors[0] = geosphere->GetColor(pt, height, norm);
			}
			break;
		case 1: {
			p = ctx->edgeLen-1;
			x1 = vertices[p-1];
			x2 = ev2[0];
			y1 = ev[ctx->edgeLen-1];
			y2 = vertices[p + ctx->edgeLen];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[p] = norm;
			// make color
			const vector3d pt = GetSpherePoint(p*ctx->frac, 0);
		//	const double height = colors[p].x;
			const double height = geosphere->GetHeight(pt);
			colors[p] = geosphere->GetColor(pt, height, norm);
			}
			break;
		case 2: {
			p = ctx->edgeLen-1;
			x1 = vertices[(p-1) + p*ctx->edgeLen];
			x2 = ev[ctx->edgeLen-1];
			y1 = vertices[p + (p-1)*ctx->edgeLen];
			y2 = ev2[0];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[p + p*ctx->edgeLen] = norm;
			// make color
			const vector3d pt = GetSpherePoint(p*ctx->frac, p*ctx->frac);
		//	const double height = colors[p + p*ctx->edgeLen].x;
			const double height = geosphere->GetHeight(pt);
			colors[p + p*ctx->edgeLen] = geosphere->GetColor(pt, height, norm);
			}
			break;
		case 3: {
			p = ctx->edgeLen-1;
			x1 = ev2[0];
			x2 = vertices[1 + p*ctx->edgeLen];
			y1 = vertices[(p-1)*ctx->edgeLen];
			y2 = ev[ctx->edgeLen-1];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[p*ctx->edgeLen] = norm;
			// make color
			const vector3d pt = GetSpherePoint(0, p*ctx->frac);
		//	const double height = colors[p*ctx->edgeLen].x;
			const double height = geosphere->GetHeight(pt);
			colors[p*ctx->edgeLen] = geosphere->GetColor(pt, height, norm);
			}
			break;
		}
	}

	void FixCornerNormalsByEdge(int edge, vector3d *ev) {
		vector3d ev2[GEOPATCH_MAX_EDGELEN];
		vector3d x1, x2, y1, y2;
		/* XXX All these 'if's have an unfinished else, when a neighbour
		 * of our size doesn't exist and instead we must look at a bigger tile.
		 * But let's just leave it for the mo because it is a pain.
		 * See comment in OnEdgeFriendChanged() */
		switch (edge) {
		case 0:
			if (edgeFriend[3]) {
				int we_are = edgeFriend[3]->GetEdgeIdxOf(this);
				edgeFriend[3]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
				MakeCornerNormal<0>(ev2, ev);
			}
			if (edgeFriend[1]) {
				int we_are = edgeFriend[1]->GetEdgeIdxOf(this);
				edgeFriend[1]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
				MakeCornerNormal<1>(ev, ev2);
			}
			break;
		case 1:
			if (edgeFriend[0]) {
				int we_are = edgeFriend[0]->GetEdgeIdxOf(this);
				edgeFriend[0]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
				MakeCornerNormal<1>(ev2, ev);
			}
			if (edgeFriend[2]) {
				int we_are = edgeFriend[2]->GetEdgeIdxOf(this);
				edgeFriend[2]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
				MakeCornerNormal<2>(ev, ev2);
			}
			break;
		case 2:
			if (edgeFriend[1]) {
				int we_are = edgeFriend[1]->GetEdgeIdxOf(this);
				edgeFriend[1]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
				MakeCornerNormal<2>(ev2, ev);
			}
			if (edgeFriend[3]) {
				int we_are = edgeFriend[3]->GetEdgeIdxOf(this);
				edgeFriend[3]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
				MakeCornerNormal<3>(ev, ev2);
			}
			break;
		case 3:
			if (edgeFriend[2]) {
				int we_are = edgeFriend[2]->GetEdgeIdxOf(this);
				edgeFriend[2]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
				MakeCornerNormal<3>(ev2, ev);
			}
			if (edgeFriend[0]) {
				int we_are = edgeFriend[0]->GetEdgeIdxOf(this);
				edgeFriend[0]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
				MakeCornerNormal<0>(ev, ev2);
			}
			break;
		}
				
	}

	void GenerateEdgeNormalsAndColors() {
		vector3d ev[4][GEOPATCH_MAX_EDGELEN];
		bool doneEdge[4];
		memset(doneEdge, 0, sizeof(doneEdge));
		for (int i=0; i<4; i++) {
			GeoPatch *e = edgeFriend[i];
			if (e) {
				int we_are = e->GetEdgeIdxOf(this);
				e->GetEdgeMinusOneVerticesFlipped(we_are, ev[i]);
			} else {
				assert(parent->edgeFriend[i]);
				doneEdge[i] = true;
				// parent has valid edge, so take our
				// bit of that, interpolated.
				FixEdgeFromParentInterpolated(i);
				// XXX needed for corners... probably not
				// correct
				ctx->GetEdge(vertices, i, ev[i]);
			}
		}
	
		MakeCornerNormal<0>(ev[3], ev[0]);
		MakeCornerNormal<1>(ev[0], ev[1]);
		MakeCornerNormal<2>(ev[1], ev[2]);
		MakeCornerNormal<3>(ev[2], ev[3]);

		for (int i=0; i<4; i++) if(!doneEdge[i]) FixEdgeNormals(i, ev[i]);
	}

	/* in patch surface coords, [0,1] */
	vector3d GetSpherePoint(double x, double y) {
		return (v[0] + x*(1.0-y)*(v[1]-v[0]) +
			    x*y*(v[2]-v[0]) +
			    (1.0-x)*y*(v[3]-v[0])).Normalized();
	}

	/** Generates full-detail vertices, and also non-edge normals and
	 * colors */
	void GenerateMesh() {
		centroid = clipCentroid.Normalized();
		centroid = (1.0 + geosphere->GetHeight(centroid)) * centroid;
		vector3d *vts = vertices;
		vector3d *col = colors;
		double xfrac;
		double yfrac = 0;
		for (int y=0; y<ctx->edgeLen; y++) {
			xfrac = 0;
			for (int x=0; x<ctx->edgeLen; x++) {
				vector3d p = GetSpherePoint(xfrac, yfrac);
				double height = geosphere->GetHeight(p);
				*(vts++) = p * (height + 1.0);
				// remember this -- we will need it later
				(col++)->x = height;
				xfrac += ctx->frac;
			}
			yfrac += ctx->frac;
		}
		assert(vts == &vertices[ctx->NUMVERTICES()]);
		// Generate normals & colors for non-edge vertices since they never change
		for (int y=1; y<ctx->edgeLen-1; y++) {
			for (int x=1; x<ctx->edgeLen-1; x++) {
				// normal
				vector3d x1 = vertices[x-1 + y*ctx->edgeLen];
				vector3d x2 = vertices[x+1 + y*ctx->edgeLen];
				vector3d y1 = vertices[x + (y-1)*ctx->edgeLen];
				vector3d y2 = vertices[x + (y+1)*ctx->edgeLen];

				vector3d n = (x2-x1).Cross(y2-y1);
				normals[x + y*ctx->edgeLen] = n.Normalized();
				// color
				vector3d p = GetSpherePoint(x*ctx->frac, y*ctx->frac);
				vector3d &col_r = colors[x + y*ctx->edgeLen];
				const double height = col_r.x;
				const vector3d &norm = normals[x + y*ctx->edgeLen];
				col_r = geosphere->GetColor(p, height, norm);
			}
		}
		
	}
	void OnEdgeFriendChanged(int edge, GeoPatch *e) {
		edgeFriend[edge] = e;
		vector3d ev[GEOPATCH_MAX_EDGELEN];
		int we_are = e->GetEdgeIdxOf(this);
		e->GetEdgeMinusOneVerticesFlipped(we_are, ev);
		/* now we have a valid edge, fix the edge vertices */
		if (edge == 0) {
			for (int x=0; x<ctx->edgeLen; x++) {
				vector3d p = GetSpherePoint(x * ctx->frac, 0);
				double height = geosphere->GetHeight(p);
				vertices[x] = p * (height + 1.0);
				// XXX These bounds checks in each edge case are
				// only necessary while the "All these 'if's"
				// comment in FixCOrnerNormalsByEdge stands
				if ((x>0) && (x<ctx->edgeLen-1)) {
					colors[x].x = height;
				}
			}
		} else if (edge == 1) {
			for (int y=0; y<ctx->edgeLen; y++) {
				vector3d p = GetSpherePoint(1.0, y * ctx->frac);
				double height = geosphere->GetHeight(p);
				int pos = (ctx->edgeLen-1) + y*ctx->edgeLen;
				vertices[pos] = p * (height + 1.0);
				if ((y>0) && (y<ctx->edgeLen-1)) {
					colors[pos].x = height;
				}
			}
		} else if (edge == 2) {
			for (int x=0; x<ctx->edgeLen; x++) {
				vector3d p = GetSpherePoint(x * ctx->frac, 1.0);
				double height = geosphere->GetHeight(p);
				int pos = x + (ctx->edgeLen-1)*ctx->edgeLen;
				vertices[pos] = p * (height + 1.0);
				if ((x>0) && (x<ctx->edgeLen-1)) {
					colors[pos].x = height;
				}
			}
		} else {
			for (int y=0; y<ctx->edgeLen; y++) {
				vector3d p = GetSpherePoint(0, y * ctx->frac);
				double height = geosphere->GetHeight(p);
				int pos = y * ctx->edgeLen;
				vertices[pos] = p * (height + 1.0);
				if ((y>0) && (y<ctx->edgeLen-1)) {
					colors[pos].x = height;
				}
			}
		}

		FixEdgeNormals(edge, ev);
		FixCornerNormalsByEdge(edge, ev);
		UpdateVBOs();

		if (kids[0]) {
			if (edge == 0) {
				kids[0]->FixEdgeFromParentInterpolated(0);
				kids[0]->UpdateVBOs();
				kids[1]->FixEdgeFromParentInterpolated(0);
				kids[1]->UpdateVBOs();
			} else if (edge == 1) {
				kids[1]->FixEdgeFromParentInterpolated(1);
				kids[1]->UpdateVBOs();
				kids[2]->FixEdgeFromParentInterpolated(1);
				kids[2]->UpdateVBOs();
			} else if (edge == 2) {
				kids[2]->FixEdgeFromParentInterpolated(2);
				kids[2]->UpdateVBOs();
				kids[3]->FixEdgeFromParentInterpolated(2);
				kids[3]->UpdateVBOs();
			} else {
				kids[3]->FixEdgeFromParentInterpolated(3);
				kids[3]->UpdateVBOs();
				kids[0]->FixEdgeFromParentInterpolated(3);
				kids[0]->UpdateVBOs();
			}
		}
	}
	void NotifyEdgeFriendSplit(GeoPatch *e) {
		int idx = GetEdgeIdxOf(e);
		int we_are = e->GetEdgeIdxOf(this);
		if (!kids[0]) return;
		// match e's new kids to our own... :/
		kids[idx]->OnEdgeFriendChanged(idx, e->kids[(we_are+1)%4]);
		kids[(idx+1)%4]->OnEdgeFriendChanged(idx, e->kids[we_are]);
	}
	
	void NotifyEdgeFriendDeleted(GeoPatch *e) {
		int idx = GetEdgeIdxOf(e);
		edgeFriend[idx] = 0;
		if (!parent) return;
		if (parent->edgeFriend[idx]) {
			FixEdgeFromParentInterpolated(idx);
			UpdateVBOs();
		} else {
			// XXX TODO XXX
			// Bad. not fixing up edges in this case!!!
		}
	}

	GeoPatch *GetEdgeFriendForKid(int kid, int edge) {
		GeoPatch *e = edgeFriend[edge];
		if (!e) return 0;
		//assert (e);// && (e->m_depth >= m_depth));
		const int we_are = e->GetEdgeIdxOf(this);
		// neighbour patch has not split yet (is at depth of this patch), so kids of this patch do
		// not have same detail level neighbours yet
		if (edge == kid) return e->kids[(we_are+1)%4];
		else return e->kids[we_are];
	}
	
	void Render(vector3d &campos, const Frustum &frustum) {
		PiVerify(SDL_mutexP(m_kidsLock)==0);
		if (kids[0]) {
			for (int i=0; i<4; i++) kids[i]->Render(campos, frustum);
			SDL_mutexV(m_kidsLock);
		} else {
			SDL_mutexV(m_kidsLock);
			_UpdateVBOs();

			if (!frustum.TestPoint(clipCentroid, clipRadius))
				return;

			vector3d relpos = clipCentroid - campos;
			glPushMatrix();
			glTranslated(relpos.x, relpos.y, relpos.z);

			Pi::statSceneTris += 2*(ctx->edgeLen-1)*(ctx->edgeLen-1);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);

			glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
			glVertexPointer(3, GL_FLOAT, sizeof(VBOVertex), 0);
			glNormalPointer(GL_FLOAT, sizeof(VBOVertex), reinterpret_cast<void *>(3*sizeof(float)));
			glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VBOVertex), reinterpret_cast<void *>(6*sizeof(float)));
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, ctx->indices_vbo);
			glDrawRangeElements(GL_TRIANGLES, 0, ctx->NUMVERTICES()-1, ctx->VBO_COUNT_MID_IDX(), GL_UNSIGNED_SHORT, reinterpret_cast<void*>(ctx->IDX_VBO_MAIN_OFFSET()));
			for (int i=0; i<4; i++) {
				if (edgeFriend[i]) {
					glDrawRangeElements(GL_TRIANGLES, s_hiMinIdx[i], s_hiMaxIdx[i], ctx->VBO_COUNT_HI_EDGE(), GL_UNSIGNED_SHORT, reinterpret_cast<void*>(ctx->IDX_VBO_HI_OFFSET(i)));
				} else {
					glDrawRangeElements(GL_TRIANGLES, s_loMinIdx[i], s_loMaxIdx[i], ctx->VBO_COUNT_LO_EDGE(), GL_UNSIGNED_SHORT, reinterpret_cast<void*>(ctx->IDX_VBO_LO_OFFSET(i)));
				}
			}
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);

			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
			glPopMatrix();
		}
	}

	void LODUpdate(vector3d &campos) {
		// if we've been asked to abort then get out as quickly as possible
		// this function is recursive so we might be very deep. this is about
		// as fast as we can go
		SDL_mutexP(geosphere->m_abortLock);
		bool abort = geosphere->m_abort;
		SDL_mutexV(geosphere->m_abortLock);
		if (abort)
			return;
				
		bool canSplit = true;
		for (int i=0; i<4; i++) {
			if (!edgeFriend[i]) { canSplit = false; break; }
			if (edgeFriend[i] && (edgeFriend[i]->m_depth < m_depth)) {
				canSplit = false;
				break;
			}
		}
		if (!(canSplit && (m_depth < GEOPATCH_MAX_DEPTH) &&
		    ((campos - centroid).Length() < m_roughLength)))
			canSplit = false;
		// always split at first level
		if (!parent) canSplit = true;
		//printf(campos.Length());

		bool canMerge = true;

		if (canSplit) {
			if (!kids[0]) {
				vector3d v01, v12, v23, v30, cn;
				cn = centroid.Normalized();			
				v01 = (v[0]+v[1]).Normalized();
				v12 = (v[1]+v[2]).Normalized();
				v23 = (v[2]+v[3]).Normalized();
				v30 = (v[3]+v[0]).Normalized();
				GeoPatch *_kids[4];
				_kids[0] = new GeoPatch(ctx, geosphere, v[0], v01, cn, v30, m_depth+1);
				_kids[1] = new GeoPatch(ctx, geosphere, v01, v[1], v12, cn, m_depth+1);
				_kids[2] = new GeoPatch(ctx, geosphere, cn, v12, v[2], v23, m_depth+1);
				_kids[3] = new GeoPatch(ctx, geosphere, v30, cn, v23, v[3], m_depth+1);
				// hm.. edges. Not right to pass this
				// edgeFriend...
				_kids[0]->edgeFriend[0] = GetEdgeFriendForKid(0, 0);
				_kids[0]->edgeFriend[1] = _kids[1];
				_kids[0]->edgeFriend[2] = _kids[3];
				_kids[0]->edgeFriend[3] = GetEdgeFriendForKid(0, 3);
				_kids[1]->edgeFriend[0] = GetEdgeFriendForKid(1, 0);
				_kids[1]->edgeFriend[1] = GetEdgeFriendForKid(1, 1);
				_kids[1]->edgeFriend[2] = _kids[2];
				_kids[1]->edgeFriend[3] = _kids[0];
				_kids[2]->edgeFriend[0] = _kids[1];
				_kids[2]->edgeFriend[1] = GetEdgeFriendForKid(2, 1);
				_kids[2]->edgeFriend[2] = GetEdgeFriendForKid(2, 2);
				_kids[2]->edgeFriend[3] = _kids[3];
				_kids[3]->edgeFriend[0] = _kids[0];
				_kids[3]->edgeFriend[1] = _kids[2];
				_kids[3]->edgeFriend[2] = GetEdgeFriendForKid(3, 2);
				_kids[3]->edgeFriend[3] = GetEdgeFriendForKid(3, 3);
				_kids[0]->parent = _kids[1]->parent = _kids[2]->parent = _kids[3]->parent = this;
				for (int i=0; i<4; i++) _kids[i]->GenerateMesh();
				PiVerify(SDL_mutexP(m_kidsLock)==0);
				for (int i=0; i<4; i++) kids[i] = _kids[i];
				for (int i=0; i<4; i++) edgeFriend[i]->NotifyEdgeFriendSplit(this);
				for (int i=0; i<4; i++) {
					kids[i]->GenerateEdgeNormalsAndColors();
					kids[i]->UpdateVBOs();
				}
				PiVerify(SDL_mutexV(m_kidsLock)!=-1);
			}
			for (int i=0; i<4; i++) kids[i]->LODUpdate(campos);
		} else {
			if (canMerge && kids[0]) {
				PiVerify(SDL_mutexP(m_kidsLock)==0);
				for (int i=0; i<4; i++) { delete kids[i]; kids[i] = 0; }
				PiVerify(SDL_mutexV(m_kidsLock)!=-1);
			}
		}
	}
};

static const int geo_sphere_edge_friends[6][4] = {
	{ 3, 4, 1, 2 },
	{ 0, 4, 5, 2 },
	{ 0, 1, 5, 3 },
	{ 0, 2, 5, 4 },
	{ 0, 3, 5, 1 },
	{ 1, 4, 3, 2 }
};

static std::vector<GeoSphere*> s_allGeospheres;
static std::deque<GeoSphere*> s_geosphereUpdateQueue;
static GeoSphere* s_currentlyUpdatingGeoSphere = 0;
static SDL_mutex *s_geosphereUpdateQueueLock = 0;
static SDL_Thread *s_updateThread = 0;

static bool s_exitFlag = false;

/* Thread that updates geosphere level of detail thingies */
int GeoSphere::UpdateLODThread(void *data)
{
	bool done = false;

	while (!done) {

		// pull the next GeoSphere off the queue
		SDL_mutexP(s_geosphereUpdateQueueLock);

		// check for exit. doing it here to avoid needing another lock
		if (s_exitFlag) {
			done = true;
			SDL_mutexV(s_geosphereUpdateQueueLock);
			break;
		}

		if (! s_geosphereUpdateQueue.empty()) {
			s_currentlyUpdatingGeoSphere = s_geosphereUpdateQueue.front();
			s_geosphereUpdateQueue.pop_front();
		} else
			s_currentlyUpdatingGeoSphere = 0;

		if (s_currentlyUpdatingGeoSphere) {
			GeoSphere *gs = s_currentlyUpdatingGeoSphere;
			// overlap locks to ensure gs doesn't die before we've locked it
			SDL_mutexP(gs->m_updateLock);
			SDL_mutexV(s_geosphereUpdateQueueLock);

			// update the patches
			for (int n=0; n<6; n++)
				gs->m_patches[n]->LODUpdate(gs->m_tempCampos);

			// overlap locks again
			SDL_mutexP(s_geosphereUpdateQueueLock);
			assert(s_currentlyUpdatingGeoSphere == gs);
			s_currentlyUpdatingGeoSphere = 0;
			SDL_mutexV(s_geosphereUpdateQueueLock);

			SDL_mutexV(gs->m_updateLock);
		} else {
			// if there's nothing in the update queue, just sleep for a bit before checking it again
			// XXX could use a semaphore instead, but polling is probably ok
			SDL_mutexV(s_geosphereUpdateQueueLock);
			SDL_Delay(10);
		}
	}

	return 0;
}

void GeoSphere::Init()
{
	s_geosphereSurfaceShader[0] = new GeosphereShader("geosphere", "#define NUM_LIGHTS 1\n");
	s_geosphereSurfaceShader[1] = new GeosphereShader("geosphere", "#define NUM_LIGHTS 2\n");
	s_geosphereSurfaceShader[2] = new GeosphereShader("geosphere", "#define NUM_LIGHTS 3\n");
	s_geosphereSurfaceShader[3] = new GeosphereShader("geosphere", "#define NUM_LIGHTS 4\n");
	s_geosphereSkyShader[0] = new GeosphereShader("geosphere_sky", "#define NUM_LIGHTS 1\n");
	s_geosphereSkyShader[1] = new GeosphereShader("geosphere_sky", "#define NUM_LIGHTS 2\n");
	s_geosphereSkyShader[2] = new GeosphereShader("geosphere_sky", "#define NUM_LIGHTS 3\n");
	s_geosphereSkyShader[3] = new GeosphereShader("geosphere_sky", "#define NUM_LIGHTS 4\n");
	s_geosphereStarShader = new GeosphereShader("geosphere_star");
	s_geosphereDimStarShader[0] = new GeosphereShader("geosphere_star", "#define DIM\n#define NUM_LIGHTS 1\n");
	s_geosphereDimStarShader[1] = new GeosphereShader("geosphere_star", "#define DIM\n#define NUM_LIGHTS 2\n");
	s_geosphereDimStarShader[2] = new GeosphereShader("geosphere_star", "#define DIM\n#define NUM_LIGHTS 3\n");
	s_geosphereDimStarShader[3] = new GeosphereShader("geosphere_star", "#define DIM\n#define NUM_LIGHTS 4\n");
	s_geosphereUpdateQueueLock = SDL_CreateMutex();

	s_patchContext.Reset(new GeoPatchContext(detail_edgeLen[Pi::detail.planets > 4 ? 4 : Pi::detail.planets]));
	assert(s_patchContext->edgeLen <= GEOPATCH_MAX_EDGELEN);

#ifdef GEOSPHERE_USE_THREADING
	s_updateThread = SDL_CreateThread(&GeoSphere::UpdateLODThread, 0);
#endif /* GEOSPHERE_USE_THREADING */
}

void GeoSphere::Uninit()
{
#ifdef GEOSPHERE_USE_THREADING
	// instruct the thread to exit
	assert(s_geosphereUpdateQueue.empty());
	SDL_mutexP(s_geosphereUpdateQueueLock);
	s_exitFlag = true;
	SDL_mutexV(s_geosphereUpdateQueueLock);

	SDL_WaitThread(s_updateThread, 0);
#endif /* GEOSPHERE_USE_THREADING */
	
	assert (s_patchContext.Unique());
	s_patchContext.Reset();

	SDL_DestroyMutex(s_geosphereUpdateQueueLock);
	for (int i=0; i<4; i++) delete s_geosphereDimStarShader[i];
	delete s_geosphereStarShader;
	for (int i=0; i<4; i++) delete s_geosphereSkyShader[i];
	for (int i=0; i<4; i++) delete s_geosphereSurfaceShader[i];
}

static void print_info(const SBody *sbody, const Terrain *terrain)
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

	// cancel all queued updates
	SDL_mutexP(s_geosphereUpdateQueueLock);
	s_geosphereUpdateQueue.clear();
	GeoSphere *gs = s_currentlyUpdatingGeoSphere;
	SDL_mutexV(s_geosphereUpdateQueueLock);

	// if a terrain is currently being updated, then abort the update
	if (gs) {
		SDL_mutexP(gs->m_abortLock);
		gs->m_abort = true;
		SDL_mutexV(gs->m_abortLock);
	}

	// reinit the geosphere terrain data
	for(std::vector<GeoSphere*>::iterator i = s_allGeospheres.begin();
			i != s_allGeospheres.end(); ++i) {

		// we need the update lock so we don't delete working data out from
		// under the thread. it should finish very quickly since we told it to
		// abort quickly
		SDL_mutexP((*i)->m_updateLock);

		for (int p=0; p<6; p++) {
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

		// clear the abort for the next run (with the new settings)
		(*i)->m_abort = false;

		// finished update
		SDL_mutexV((*i)->m_updateLock);
	}
}

#define GEOSPHERE_TYPE	(m_sbody->type)

GeoSphere::GeoSphere(const SBody *body)
{
	m_terrain = Terrain::InstanceTerrain(body);
	print_info(body, m_terrain);

	m_vbosToDestroyLock = SDL_CreateMutex();
	m_sbody = body;
	memset(m_patches, 0, 6*sizeof(GeoPatch*));

	m_updateLock = SDL_CreateMutex();
	m_abortLock = SDL_CreateMutex();
	m_abort = false;

	s_allGeospheres.push_back(this);
}

GeoSphere::~GeoSphere()
{
	// tell the thread to finish up with this geosphere
	SDL_mutexP(m_abortLock);
	m_abort = true;
	SDL_mutexV(m_abortLock);

	SDL_mutexP(s_geosphereUpdateQueueLock);
	assert(std::count(s_allGeospheres.begin(), s_allGeospheres.end(), this) <= 1);
	s_geosphereUpdateQueue.erase(
		std::remove(s_geosphereUpdateQueue.begin(), s_geosphereUpdateQueue.end(), this),
		s_geosphereUpdateQueue.end());
	SDL_mutexV(s_geosphereUpdateQueueLock);

	// wait until it completes update
	SDL_mutexP(m_updateLock);
	SDL_mutexV(m_updateLock);

	// update thread should not be able to access us now, so we can safely continue to delete
	assert(std::count(s_allGeospheres.begin(), s_allGeospheres.end(), this) == 1);
	s_allGeospheres.erase(std::find(s_allGeospheres.begin(), s_allGeospheres.end(), this));

	SDL_DestroyMutex(m_abortLock);
	SDL_DestroyMutex(m_updateLock);

	for (int i=0; i<6; i++) if (m_patches[i]) delete m_patches[i];
	DestroyVBOs();
	SDL_DestroyMutex(m_vbosToDestroyLock);

	delete m_terrain;
}

void GeoSphere::AddVBOToDestroy(GLuint vbo)
{
	SDL_mutexP(m_vbosToDestroyLock);
	m_vbosToDestroy.push_back(vbo);
	SDL_mutexV(m_vbosToDestroyLock);
}

void GeoSphere::DestroyVBOs()
{
	SDL_mutexP(m_vbosToDestroyLock);
	for (std::list<GLuint>::iterator i = m_vbosToDestroy.begin();
			i != m_vbosToDestroy.end(); ++i) {
		glDeleteBuffersARB(1, &(*i));
	}
	m_vbosToDestroy.clear();
	SDL_mutexV(m_vbosToDestroyLock);
}

void GeoSphere::BuildFirstPatches()
{
	// generate initial wank
	vector3d p1(1,1,1);
	vector3d p2(-1,1,1);
	vector3d p3(-1,-1,1);
	vector3d p4(1,-1,1);
	vector3d p5(1,1,-1);
	vector3d p6(-1,1,-1);
	vector3d p7(-1,-1,-1);
	vector3d p8(1,-1,-1);
	p1 = p1.Normalized();
	p2 = p2.Normalized();
	p3 = p3.Normalized();
	p4 = p4.Normalized();
	p5 = p5.Normalized();
	p6 = p6.Normalized();
	p7 = p7.Normalized();
	p8 = p8.Normalized();

	m_patches[0] = new GeoPatch(s_patchContext, this, p1, p2, p3, p4, 0);
	m_patches[1] = new GeoPatch(s_patchContext, this, p4, p3, p7, p8, 0);
	m_patches[2] = new GeoPatch(s_patchContext, this, p1, p4, p8, p5, 0);
	m_patches[3] = new GeoPatch(s_patchContext, this, p2, p1, p5, p6, 0);
	m_patches[4] = new GeoPatch(s_patchContext, this, p3, p2, p6, p7, 0);
	m_patches[5] = new GeoPatch(s_patchContext, this, p8, p7, p6, p5, 0);
	for (int i=0; i<6; i++) {
		for (int j=0; j<4; j++) {
			m_patches[i]->edgeFriend[j] = m_patches[geo_sphere_edge_friends[i][j]];
		}
	}
	for (int i=0; i<6; i++) m_patches[i]->GenerateMesh();
	for (int i=0; i<6; i++) m_patches[i]->GenerateEdgeNormalsAndColors();
	for (int i=0; i<6; i++) m_patches[i]->UpdateVBOs();
}

static const float g_ambient[4] = { 0, 0, 0, 1.0 };

static void DrawAtmosphereSurface(Renderer *renderer, const vector3d &campos, float rad, Material *mat)
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
	VertexArray va(ATTRIB_POSITION);
	va.Add(vector3f(0.f, 1.f, 0.f));
	for (int i=0; i<=LONG_SEGS; i++) {
		va.Add(vector3f(
			sin(latDiff)*sinCosTable[i][0],
			cos(latDiff),
			-sin(latDiff)*sinCosTable[i][1]));
	}
	renderer->DrawTriangles(&va, mat, TRIANGLE_FAN);

	/* and wound latitudinal strips */
	double lat = latDiff;
	for (int j=1; j<LAT_SEGS; j++, lat += latDiff) {
		VertexArray v(ATTRIB_POSITION);
		float cosLat = cos(lat);
		float sinLat = sin(lat);
		float cosLat2 = cos(lat+latDiff);
		float sinLat2 = sin(lat+latDiff);
		for (int i=0; i<=LONG_SEGS; i++) {
			v.Add(vector3f(sinLat*sinCosTable[i][0], cosLat, -sinLat*sinCosTable[i][1]));
			v.Add(vector3f(sinLat2*sinCosTable[i][0], cosLat2, -sinLat2*sinCosTable[i][1]));
		}
		renderer->DrawTriangles(&v, mat, TRIANGLE_STRIP);
	}

	glPopMatrix();
}

void GeoSphere::Render(Renderer *renderer, vector3d campos, const float radius, const float scale) {
	glPushMatrix();
	glTranslated(-campos.x, -campos.y, -campos.z);
	Frustum frustum = Frustum::FromGLState();

	const float atmosRadius = ATMOSPHERE_RADIUS;
	
	// no frustum test of entire geosphere, since Space::Render does this
	// for each body using its GetBoundingRadius() value
	GeosphereShader *shader = 0;

	if (AreShadersEnabled()) {
		Color atmosCol;
		double atmosDensity;
		matrix4x4d modelMatrix;
		glGetDoublev (GL_MODELVIEW_MATRIX, &modelMatrix[0]);
		vector3d center = modelMatrix * vector3d(0.0, 0.0, 0.0);
		
		m_sbody->GetAtmosphereFlavor(&atmosCol, &atmosDensity);
		atmosDensity *= 0.00005;

		if (atmosDensity > 0.0) {
			shader = s_geosphereSkyShader[Graphics::State::GetNumLights()-1];
			shader->Use();
			shader->set_geosphereScale(scale);
			shader->set_geosphereAtmosTopRad(atmosRadius*radius/scale);
			shader->set_geosphereAtmosFogDensity(atmosDensity);
			shader->set_atmosColor(atmosCol.r, atmosCol.g, atmosCol.b, atmosCol.a);
			shader->set_geosphereCenter(center.x, center.y, center.z);

			Material atmoMat;
			atmoMat.shader = shader;
			
			renderer->SetBlendMode(BLEND_ALPHA_ONE);
			renderer->SetDepthWrite(false);
			// make atmosphere sphere slightly bigger than required so
			// that the edges of the pixel shader atmosphere jizz doesn't
			// show ugly polygonal angles
			DrawAtmosphereSurface(renderer, campos, atmosRadius*1.01, &atmoMat);
			renderer->SetDepthWrite(true);
			renderer->SetBlendMode(BLEND_SOLID);
		}

		if ((m_sbody->type == SBody::TYPE_BROWN_DWARF) || 
			(m_sbody->type == SBody::TYPE_STAR_M)){
			shader = s_geosphereDimStarShader[Graphics::State::GetNumLights()-1];
			shader->Use();
		}
		else if (m_sbody->GetSuperType() == SBody::SUPERTYPE_STAR) {
			shader = s_geosphereStarShader;
			shader->Use();
		} else {
			shader = s_geosphereSurfaceShader[Graphics::State::GetNumLights()-1];
			shader->Use();
			shader->set_geosphereScale(scale);
			shader->set_geosphereAtmosTopRad(atmosRadius*radius/scale);
			shader->set_geosphereAtmosFogDensity(atmosDensity);
			shader->set_atmosColor(atmosCol.r, atmosCol.g, atmosCol.b, atmosCol.a);
			shader->set_geosphereCenter(center.x, center.y, center.z);
		}
	}
	glPopMatrix();

	if (!m_patches[0]) BuildFirstPatches();

	const float black[4] = { 0,0,0,0 };
	Color ambient;
	float emission[4] = { 0,0,0,0 };

	// save old global ambient
	// XXX add GetAmbient to renderer or save ambient in scene? (Space)
	Color oldAmbient;
	glGetFloatv(GL_LIGHT_MODEL_AMBIENT, oldAmbient);

	float b = AreShadersEnabled() ? 2.0f : 1.5f; //XXX ??

	if ((m_sbody->GetSuperType() == SBody::SUPERTYPE_STAR) || (m_sbody->type == SBody::TYPE_BROWN_DWARF)) {
		// stars should emit light and terrain should be visible from distance
		ambient.r = ambient.g = ambient.b = 0.2f;
		ambient.a = 1.0f;
		emission[0] = StarSystem::starRealColors[m_sbody->type][0] * 0.5f * b;
		emission[1] = StarSystem::starRealColors[m_sbody->type][1] * 0.5f * b;
		emission[2] = StarSystem::starRealColors[m_sbody->type][2] * 0.5f * b;
		emission[3] = 0.5f;
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
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv (GL_FRONT, GL_SPECULAR, black);
	glMaterialfv (GL_FRONT, GL_EMISSION, emission);
	glEnable(GL_COLOR_MATERIAL);

//	glLineWidth(1.0);
//	glPolygonMode(GL_FRONT, GL_LINE);
	for (int i=0; i<6; i++) {
		m_patches[i]->Render(campos, frustum);
	}
	if (shader) shader->Unuse();

	glDisable(GL_COLOR_MATERIAL);
	renderer->SetAmbientColor(oldAmbient);

	// if the update thread has deleted any geopatches, destroy the vbos
	// associated with them
	DestroyVBOs();
		/*this->m_tempCampos = campos;
		UpdateLODThread(this);
		return;*/

	SDL_mutexP(s_geosphereUpdateQueueLock);
	bool onQueue =
		(std::find(s_geosphereUpdateQueue.begin(), s_geosphereUpdateQueue.end(), this)
			!= s_geosphereUpdateQueue.end());
	// put ourselves on the update queue, unless we're already there or already being updated
	if (!onQueue && (s_currentlyUpdatingGeoSphere != this)) {
		this->m_tempCampos = campos;
		s_geosphereUpdateQueue.push_back(this);
	}
	SDL_mutexV(s_geosphereUpdateQueueLock);

#ifndef GEOSPHERE_USE_THREADING
	m_tempCampos = campos;
	_UpdateLODs();
#endif /* !GEOSPHERE_USE_THREADING */
}

