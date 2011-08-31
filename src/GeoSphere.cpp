#include "libs.h"
#include "GeoSphere.h"
#include "perlin.h"
#include "Pi.h"
#include "StarSystem.h"
#include "render/Render.h"

// tri edge lengths
#define GEOPATCH_SUBDIVIDE_AT_CAMDIST	5.0
#define GEOPATCH_MAX_DEPTH	15
// must be an odd number
//#define GEOPATCH_EDGELEN	15
#define GEOPATCH_NUMVERTICES	(GEOPATCH_EDGELEN*GEOPATCH_EDGELEN)
#define GEOSPHERE_USE_THREADING

int GEOPATCH_EDGELEN = 15;
static const int GEOPATCH_MAX_EDGELEN = 55;
static double GEOPATCH_FRAC;
int GeoSphere::s_vtxGenCount = 0;


#define PRINT_VECTOR(_v) printf("%f,%f,%f\n", (_v).x, (_v).y, (_v).z);

SHADER_CLASS_BEGIN(GeosphereShader)
	SHADER_UNIFORM_VEC4(atmosColor)
	SHADER_UNIFORM_FLOAT(geosphereScale)
	SHADER_UNIFORM_FLOAT(geosphereAtmosTopRad)
	SHADER_UNIFORM_VEC3(geosphereCenter)
	SHADER_UNIFORM_FLOAT(geosphereAtmosFogDensity)
SHADER_CLASS_END()

static GeosphereShader *s_geosphereSurfaceShader[4], *s_geosphereSkyShader[4];

#pragma pack(4)
struct VBOVertex
{
	float x,y,z;
	float nx,ny,nz;
	unsigned char col[4];
	float padding;
};
#pragma pack()

#define VBO_COUNT_LO_EDGE  (3*(GEOPATCH_EDGELEN/2))
#define VBO_COUNT_HI_EDGE  (3*(GEOPATCH_EDGELEN-1))
#define VBO_COUNT_MID_IDX  (4*3*(GEOPATCH_EDGELEN-3) + 2*(GEOPATCH_EDGELEN-3)*(GEOPATCH_EDGELEN-3)*3)
//                          ^^ serrated teeth bit      ^^^ square inner bit
#define IDX_VBO_LO_OFFSET(_i) ((_i)*sizeof(unsigned short)*3*(GEOPATCH_EDGELEN/2))
#define IDX_VBO_HI_OFFSET(_i) (((_i)*sizeof(unsigned short)*VBO_COUNT_HI_EDGE)+IDX_VBO_LO_OFFSET(4))
#define IDX_VBO_MAIN_OFFSET IDX_VBO_HI_OFFSET(4)

// for glDrawRangeElements
static int s_loMinIdx[4], s_loMaxIdx[4];
static int s_hiMinIdx[4], s_hiMaxIdx[4];

class GeoPatch {
public:
	vector3d v[4];
	vector3d *vertices;
	vector3d *normals;
	vector3d *colors;
	GLuint m_vbo;
	static unsigned short *midIndices;
	static unsigned short *loEdgeIndices[4];
	static unsigned short *hiEdgeIndices[4];
	static GLuint indices_vbo;
	static VBOVertex *vbotemp;
	GeoPatch *kids[4];
	GeoPatch *parent;
	GeoPatch *edgeFriend[4]; // [0]=v01, [1]=v12, [2]=v20
	GeoSphere *geosphere;
	double m_roughLength;
	vector3d clipCentroid;
	double clipRadius;
	int m_depth;
	SDL_mutex *m_kidsLock;
	bool m_needUpdateVBOs;
	
	GeoPatch(vector3d v0, vector3d v1, vector3d v2, vector3d v3, int depth) {
		memset(this, 0, sizeof(GeoPatch));
		m_kidsLock = SDL_CreateMutex();
		v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3;
		m_depth = depth;
		clipCentroid = (v0+v1+v2+v3) * 0.25;
		clipRadius = 0;
		for (int i=0; i<4; i++) {
			clipRadius = std::max(clipRadius, (v[i]-clipCentroid).Length());
		}
		m_roughLength = GEOPATCH_SUBDIVIDE_AT_CAMDIST / pow(2.0, depth);
		m_needUpdateVBOs = false;
		normals = new vector3d[GEOPATCH_NUMVERTICES];
		vertices = new vector3d[GEOPATCH_NUMVERTICES];
		colors = new vector3d[GEOPATCH_NUMVERTICES];
	}

	static void Init() {
		GEOPATCH_FRAC = 1.0 / double(GEOPATCH_EDGELEN-1);

		if (midIndices) {
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

		{
			vbotemp = new VBOVertex[GEOPATCH_NUMVERTICES];
				
			unsigned short *idx;
			midIndices = new unsigned short[VBO_COUNT_MID_IDX];
			for (int i=0; i<4; i++) {
				loEdgeIndices[i] = new unsigned short[VBO_COUNT_LO_EDGE];
				hiEdgeIndices[i] = new unsigned short[VBO_COUNT_HI_EDGE];
			}
			/* also want vtx indices for tris not touching edge of patch */
			idx = midIndices;
			for (int x=1; x<GEOPATCH_EDGELEN-2; x++) {
				for (int y=1; y<GEOPATCH_EDGELEN-2; y++) {
					idx[0] = x + GEOPATCH_EDGELEN*y;
					idx[1] = x+1 + GEOPATCH_EDGELEN*y;
					idx[2] = x + GEOPATCH_EDGELEN*(y+1);
					idx+=3;

					idx[0] = x+1 + GEOPATCH_EDGELEN*y;
					idx[1] = x+1 + GEOPATCH_EDGELEN*(y+1);
					idx[2] = x + GEOPATCH_EDGELEN*(y+1);
					idx+=3;
				}
			}
			{
				for (int x=1; x<GEOPATCH_EDGELEN-3; x+=2) {
					// razor teeth near edge 0
					idx[0] = x + GEOPATCH_EDGELEN;
					idx[1] = x+1;
					idx[2] = x+1 + GEOPATCH_EDGELEN;
					idx+=3;
					idx[0] = x+1;
					idx[1] = x+2 + GEOPATCH_EDGELEN;
					idx[2] = x+1 + GEOPATCH_EDGELEN;
					idx+=3;
				}
				for (int x=1; x<GEOPATCH_EDGELEN-3; x+=2) {
					// near edge 2
					idx[0] = x + GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-2);
					idx[1] = x+1 + GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-2);
					idx[2] = x+1 + GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-1);
					idx+=3;
					idx[0] = x+1 + GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-2);
					idx[1] = x+2 + GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-2);
					idx[2] = x+1 + GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-1);
					idx+=3;
				}
				for (int y=1; y<GEOPATCH_EDGELEN-3; y+=2) {
					// near edge 1
					idx[0] = GEOPATCH_EDGELEN-2 + y*GEOPATCH_EDGELEN;
					idx[1] = GEOPATCH_EDGELEN-1 + (y+1)*GEOPATCH_EDGELEN;
					idx[2] = GEOPATCH_EDGELEN-2 + (y+1)*GEOPATCH_EDGELEN;
					idx+=3;
					idx[0] = GEOPATCH_EDGELEN-2 + (y+1)*GEOPATCH_EDGELEN;
					idx[1] = GEOPATCH_EDGELEN-1 + (y+1)*GEOPATCH_EDGELEN;
					idx[2] = GEOPATCH_EDGELEN-2 + (y+2)*GEOPATCH_EDGELEN;
					idx+=3;
				}
				for (int y=1; y<GEOPATCH_EDGELEN-3; y+=2) {
					// near edge 3
					idx[0] = 1 + y*GEOPATCH_EDGELEN;
					idx[1] = 1 + (y+1)*GEOPATCH_EDGELEN;
					idx[2] = (y+1)*GEOPATCH_EDGELEN;
					idx+=3;
					idx[0] = 1 + (y+1)*GEOPATCH_EDGELEN;
					idx[1] = 1 + (y+2)*GEOPATCH_EDGELEN;
					idx[2] = (y+1)*GEOPATCH_EDGELEN;
					idx+=3;
				}
			}
			// full detail edge triangles
			{
				idx = hiEdgeIndices[0];
				for (int x=0; x<GEOPATCH_EDGELEN-1; x+=2) {
					idx[0] = x; idx[1] = x+1; idx[2] = x+1 + GEOPATCH_EDGELEN;
					idx+=3;
					idx[0] = x+1; idx[1] = x+2; idx[2] = x+1 + GEOPATCH_EDGELEN;
					idx+=3;
				}
				idx = hiEdgeIndices[1];
				for (int y=0; y<GEOPATCH_EDGELEN-1; y+=2) {
					idx[0] = GEOPATCH_EDGELEN-1 + y*GEOPATCH_EDGELEN;
					idx[1] = GEOPATCH_EDGELEN-1 + (y+1)*GEOPATCH_EDGELEN;
					idx[2] = GEOPATCH_EDGELEN-2 + (y+1)*GEOPATCH_EDGELEN;
					idx+=3;
					idx[0] = GEOPATCH_EDGELEN-1 + (y+1)*GEOPATCH_EDGELEN;
					idx[1] = GEOPATCH_EDGELEN-1 + (y+2)*GEOPATCH_EDGELEN;
					idx[2] = GEOPATCH_EDGELEN-2 + (y+1)*GEOPATCH_EDGELEN;
					idx+=3;
				}
				idx = hiEdgeIndices[2];
				for (int x=0; x<GEOPATCH_EDGELEN-1; x+=2) {
					idx[0] = x + (GEOPATCH_EDGELEN-1)*GEOPATCH_EDGELEN;
					idx[1] = x+1 + (GEOPATCH_EDGELEN-2)*GEOPATCH_EDGELEN;
					idx[2] = x+1 + (GEOPATCH_EDGELEN-1)*GEOPATCH_EDGELEN;
					idx+=3;
					idx[0] = x+1 + (GEOPATCH_EDGELEN-2)*GEOPATCH_EDGELEN;
					idx[1] = x+2 + (GEOPATCH_EDGELEN-1)*GEOPATCH_EDGELEN;
					idx[2] = x+1 + (GEOPATCH_EDGELEN-1)*GEOPATCH_EDGELEN;
					idx+=3;
				}
				idx = hiEdgeIndices[3];
				for (int y=0; y<GEOPATCH_EDGELEN-1; y+=2) {
					idx[0] = y*GEOPATCH_EDGELEN;
					idx[1] = 1 + (y+1)*GEOPATCH_EDGELEN;
					idx[2] = (y+1)*GEOPATCH_EDGELEN;
					idx+=3;
					idx[0] = (y+1)*GEOPATCH_EDGELEN;
					idx[1] = 1 + (y+1)*GEOPATCH_EDGELEN;
					idx[2] = (y+2)*GEOPATCH_EDGELEN;
					idx+=3;
				}
			}
			// these edge indices are for patches with no
			// neighbour of equal or greater detail -- they reduce
			// their edge complexity by 1 division
			{
				idx = loEdgeIndices[0];
				for (int x=0; x<GEOPATCH_EDGELEN-2; x+=2) {
					idx[0] = x;
					idx[1] = x+2;
					idx[2] = x+1+GEOPATCH_EDGELEN;
					idx += 3;
				}
				idx = loEdgeIndices[1];
				for (int y=0; y<GEOPATCH_EDGELEN-2; y+=2) {
					idx[0] = (GEOPATCH_EDGELEN-1) + y*GEOPATCH_EDGELEN;
					idx[1] = (GEOPATCH_EDGELEN-1) + (y+2)*GEOPATCH_EDGELEN;
					idx[2] = (GEOPATCH_EDGELEN-2) + (y+1)*GEOPATCH_EDGELEN;
					idx += 3;
				}
				idx = loEdgeIndices[2];
				for (int x=0; x<GEOPATCH_EDGELEN-2; x+=2) {
					idx[0] = x+GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-1);
					idx[2] = x+2+GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-1);
					idx[1] = x+1+GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-2);
					idx += 3;
				}
				idx = loEdgeIndices[3];
				for (int y=0; y<GEOPATCH_EDGELEN-2; y+=2) {
					idx[0] = y*GEOPATCH_EDGELEN;
					idx[2] = (y+2)*GEOPATCH_EDGELEN;
					idx[1] = 1 + (y+1)*GEOPATCH_EDGELEN;
					idx += 3;
				}
			}
			// find min/max indices
			for (int i=0; i<4; i++) {
				s_loMinIdx[i] = s_hiMinIdx[i] = 1<<30;
				s_loMaxIdx[i] = s_hiMaxIdx[i] = 0;
				for (int j=0; j<3*(GEOPATCH_EDGELEN/2); j++) {
					if (loEdgeIndices[i][j] < s_loMinIdx[i]) s_loMinIdx[i] = loEdgeIndices[i][j];
					if (loEdgeIndices[i][j] > s_loMaxIdx[i]) s_loMaxIdx[i] = loEdgeIndices[i][j];
				}
				for (int j=0; j<VBO_COUNT_HI_EDGE; j++) {
					if (hiEdgeIndices[i][j] < s_hiMinIdx[i]) s_hiMinIdx[i] = hiEdgeIndices[i][j];
					if (hiEdgeIndices[i][j] > s_hiMaxIdx[i]) s_hiMaxIdx[i] = hiEdgeIndices[i][j];
				}
				//printf("%d:\nLo %d:%d\nHi: %d:%d\n", i, s_loMinIdx[i], s_loMaxIdx[i], s_hiMinIdx[i], s_hiMaxIdx[i]);
			}

			glGenBuffersARB(1, &indices_vbo);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, IDX_VBO_MAIN_OFFSET + sizeof(unsigned short)*VBO_COUNT_MID_IDX, 0, GL_STATIC_DRAW);
			for (int i=0; i<4; i++) {
				glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER, 
					IDX_VBO_LO_OFFSET(i),
					sizeof(unsigned short)*3*(GEOPATCH_EDGELEN/2),
					loEdgeIndices[i]);
			}
			for (int i=0; i<4; i++) {
				glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER,
					IDX_VBO_HI_OFFSET(i),
					sizeof(unsigned short)*VBO_COUNT_HI_EDGE,
					hiEdgeIndices[i]);
			}
			glBufferSubDataARB(GL_ELEMENT_ARRAY_BUFFER,
					IDX_VBO_MAIN_OFFSET,
					sizeof(unsigned short)*VBO_COUNT_MID_IDX,
					midIndices);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}

	~GeoPatch() {
		SDL_DestroyMutex(m_kidsLock);
		for (int i=0; i<4; i++) {
			if (edgeFriend[i]) edgeFriend[i]->NotifyEdgeFriendDeleted(this);
		}
		for (int i=0; i<4; i++) if (kids[i]) delete kids[i];
		delete vertices;
		delete normals;
		delete colors;
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
			glBufferDataARB(GL_ARRAY_BUFFER, sizeof(VBOVertex)*GEOPATCH_NUMVERTICES, 0, GL_DYNAMIC_DRAW);
			for (int i=0; i<GEOPATCH_NUMVERTICES; i++)
			{
				clipRadius = std::max(clipRadius, (vertices[i]-clipCentroid).Length());
				VBOVertex *pData = vbotemp + i;
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
			glBufferDataARB(GL_ARRAY_BUFFER, sizeof(VBOVertex)*GEOPATCH_NUMVERTICES, vbotemp, GL_DYNAMIC_DRAW);
			glBindBufferARB(GL_ARRAY_BUFFER, 0);
		}
	}	
	/* not quite edge, since we share edge vertices so that would be
	 * fucking pointless. one position inwards. used to make edge normals
	 * for adjacent tiles */
	void GetEdgeMinusOneVerticesFlipped(int edge, vector3d *ev) {
		if (edge == 0) {
			for (int x=0; x<GEOPATCH_EDGELEN; x++) ev[GEOPATCH_EDGELEN-1-x] = vertices[x + GEOPATCH_EDGELEN];
		} else if (edge == 1) {
			const int x = GEOPATCH_EDGELEN-2;
			for (int y=0; y<GEOPATCH_EDGELEN; y++) ev[GEOPATCH_EDGELEN-1-y] = vertices[x + y*GEOPATCH_EDGELEN];
		} else if (edge == 2) {
			const int y = GEOPATCH_EDGELEN-2;
			for (int x=0; x<GEOPATCH_EDGELEN; x++) ev[GEOPATCH_EDGELEN-1-x] = vertices[(GEOPATCH_EDGELEN-1)-x + y*GEOPATCH_EDGELEN];
		} else {
			for (int y=0; y<GEOPATCH_EDGELEN; y++) ev[GEOPATCH_EDGELEN-1-y] = vertices[1 + ((GEOPATCH_EDGELEN-1)-y)*GEOPATCH_EDGELEN];
		}
	}
	static void GetEdge(vector3d *array, int edge, vector3d *ev) {
		if (edge == 0) {
			for (int x=0; x<GEOPATCH_EDGELEN; x++) ev[x] = array[x];
		} else if (edge == 1) {
			const int x = GEOPATCH_EDGELEN-1;
			for (int y=0; y<GEOPATCH_EDGELEN; y++) ev[y] = array[x + y*GEOPATCH_EDGELEN];
		} else if (edge == 2) {
			const int y = GEOPATCH_EDGELEN-1;
			for (int x=0; x<GEOPATCH_EDGELEN; x++) ev[x] = array[(GEOPATCH_EDGELEN-1)-x + y*GEOPATCH_EDGELEN];
		} else {
			for (int y=0; y<GEOPATCH_EDGELEN; y++) ev[y] = array[0 + ((GEOPATCH_EDGELEN-1)-y)*GEOPATCH_EDGELEN];
		}
	}
	static void SetEdge(vector3d *array, int edge, const vector3d *ev) {
		if (edge == 0) {
			for (int x=0; x<GEOPATCH_EDGELEN; x++) array[x] = ev[x];
		} else if (edge == 1) {
			const int x = GEOPATCH_EDGELEN-1;
			for (int y=0; y<GEOPATCH_EDGELEN; y++) array[x + y*GEOPATCH_EDGELEN] = ev[y];
		} else if (edge == 2) {
			const int y = GEOPATCH_EDGELEN-1;
			for (int x=0; x<GEOPATCH_EDGELEN; x++) array[(GEOPATCH_EDGELEN-1)-x + y*GEOPATCH_EDGELEN] = ev[x];
		} else {
			for (int y=0; y<GEOPATCH_EDGELEN; y++) array[0 + ((GEOPATCH_EDGELEN-1)-y)*GEOPATCH_EDGELEN] = ev[y];
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
			for (x=1; x<GEOPATCH_EDGELEN-1; x++) {
				const vector3d x1 = vertices[x-1];
				const vector3d x2 = vertices[x+1];
				const vector3d y1 = ev[x];
				const vector3d y2 = vertices[x + GEOPATCH_EDGELEN];
				const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
				normals[x] = norm;
				// make color
				const vector3d p = GetSpherePoint(x*GEOPATCH_FRAC, 0);
				const double height = colors[x].x;
				colors[x] = geosphere->GetColor(p, height, norm);
			}
			break;
		case 1:
			x = GEOPATCH_EDGELEN-1;
			for (y=1; y<GEOPATCH_EDGELEN-1; y++) {
				const vector3d x1 = vertices[(x-1) + y*GEOPATCH_EDGELEN];
				const vector3d x2 = ev[y];
				const vector3d y1 = vertices[x + (y-1)*GEOPATCH_EDGELEN];
				const vector3d y2 = vertices[x + (y+1)*GEOPATCH_EDGELEN];
				const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
				normals[x + y*GEOPATCH_EDGELEN] = norm;
				// make color
				const vector3d p = GetSpherePoint(x*GEOPATCH_FRAC, y*GEOPATCH_FRAC);
				const double height = colors[x + y*GEOPATCH_EDGELEN].x;
				colors[x + y*GEOPATCH_EDGELEN] = geosphere->GetColor(p, height, norm);
	//			colors[x+y*GEOPATCH_EDGELEN] = vector3d(1,0,0);
			}
			break;
		case 2:
			y = GEOPATCH_EDGELEN-1;
			for (x=1; x<GEOPATCH_EDGELEN-1; x++) {
				const vector3d x1 = vertices[x-1 + y*GEOPATCH_EDGELEN];
				const vector3d x2 = vertices[x+1 + y*GEOPATCH_EDGELEN];
				const vector3d y1 = vertices[x + (y-1)*GEOPATCH_EDGELEN];
				const vector3d y2 = ev[GEOPATCH_EDGELEN-1-x];
				const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
				normals[x + y*GEOPATCH_EDGELEN] = norm;
				// make color
				const vector3d p = GetSpherePoint(x*GEOPATCH_FRAC, y*GEOPATCH_FRAC);
				const double height = colors[x + y*GEOPATCH_EDGELEN].x;
				colors[x + y*GEOPATCH_EDGELEN] = geosphere->GetColor(p, height, norm);
			}
			break;
		case 3:
			for (y=1; y<GEOPATCH_EDGELEN-1; y++) {
				const vector3d x1 = ev[GEOPATCH_EDGELEN-1-y];
				const vector3d x2 = vertices[1 + y*GEOPATCH_EDGELEN];
				const vector3d y1 = vertices[(y-1)*GEOPATCH_EDGELEN];
				const vector3d y2 = vertices[(y+1)*GEOPATCH_EDGELEN];
				const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
				normals[y*GEOPATCH_EDGELEN] = norm;
				// make color
				const vector3d p = GetSpherePoint(0, y*GEOPATCH_FRAC);
				const double height = colors[y*GEOPATCH_EDGELEN].x;
				colors[y*GEOPATCH_EDGELEN] = geosphere->GetColor(p, height, norm);
	//			colors[y*GEOPATCH_EDGELEN] = vector3d(0,1,0);
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
		GetEdge(parent->vertices, edge, ev);
		GetEdge(parent->normals, edge, en);
		GetEdge(parent->colors, edge, ec);

		int kid_idx = parent->GetChildIdx(this);
		if (edge == kid_idx) {
			// use first half of edge
			for (int i=0; i<=GEOPATCH_EDGELEN/2; i++) {
				ev2[i<<1] = ev[i];
				en2[i<<1] = en[i];
				ec2[i<<1] = ec[i];
			}
		} else {
			// use 2nd half of edge
			for (int i=GEOPATCH_EDGELEN/2; i<GEOPATCH_EDGELEN; i++) {
				ev2[(i-(GEOPATCH_EDGELEN/2))<<1] = ev[i];
				en2[(i-(GEOPATCH_EDGELEN/2))<<1] = en[i];
				ec2[(i-(GEOPATCH_EDGELEN/2))<<1] = ec[i];
			}
		}
		// interpolate!!
		for (int i=1; i<GEOPATCH_EDGELEN; i+=2) {
			ev2[i] = (ev2[i-1]+ev2[i+1]) * 0.5;
			en2[i] = (en2[i-1]+en2[i+1]).Normalized();
			ec2[i] = (ec2[i-1]+ec2[i+1]) * 0.5;
		}
		SetEdge(this->vertices, edge, ev2);
		SetEdge(this->normals, edge, en2);
		SetEdge(this->colors, edge, ec2);
	}

	template <int corner>
	void MakeCornerNormal(vector3d *ev, vector3d *ev2) {
		int p;
		vector3d x1,x2,y1,y2;
		switch (corner) {
		case 0: {
			x1 = ev[GEOPATCH_EDGELEN-1];
			x2 = vertices[1];
			y1 = ev2[0];
			y2 = vertices[GEOPATCH_EDGELEN];
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
			p = GEOPATCH_EDGELEN-1;
			x1 = vertices[p-1];
			x2 = ev2[0];
			y1 = ev[GEOPATCH_EDGELEN-1];
			y2 = vertices[p + GEOPATCH_EDGELEN];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[p] = norm;
			// make color
			const vector3d pt = GetSpherePoint(p*GEOPATCH_FRAC, 0);
		//	const double height = colors[p].x;
			const double height = geosphere->GetHeight(pt);
			colors[p] = geosphere->GetColor(pt, height, norm);
			}
			break;
		case 2: {
			p = GEOPATCH_EDGELEN-1;
			x1 = vertices[(p-1) + p*GEOPATCH_EDGELEN];
			x2 = ev[GEOPATCH_EDGELEN-1];
			y1 = vertices[p + (p-1)*GEOPATCH_EDGELEN];
			y2 = ev2[0];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[p + p*GEOPATCH_EDGELEN] = norm;
			// make color
			const vector3d pt = GetSpherePoint(p*GEOPATCH_FRAC, p*GEOPATCH_FRAC);
		//	const double height = colors[p + p*GEOPATCH_EDGELEN].x;
			const double height = geosphere->GetHeight(pt);
			colors[p + p*GEOPATCH_EDGELEN] = geosphere->GetColor(pt, height, norm);
			}
			break;
		case 3: {
			p = GEOPATCH_EDGELEN-1;
			x1 = ev2[0];
			x2 = vertices[1 + p*GEOPATCH_EDGELEN];
			y1 = vertices[(p-1)*GEOPATCH_EDGELEN];
			y2 = ev[GEOPATCH_EDGELEN-1];
			const vector3d norm = (x2-x1).Cross(y2-y1).Normalized();
			normals[p*GEOPATCH_EDGELEN] = norm;
			// make color
			const vector3d pt = GetSpherePoint(0, p*GEOPATCH_FRAC);
		//	const double height = colors[p*GEOPATCH_EDGELEN].x;
			const double height = geosphere->GetHeight(pt);
			colors[p*GEOPATCH_EDGELEN] = geosphere->GetColor(pt, height, norm);
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
				GetEdge(vertices, i, ev[i]);
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
		vector3d *vts = vertices;
		vector3d *col = colors;
		double xfrac;
		double yfrac = 0;
		for (int y=0; y<GEOPATCH_EDGELEN; y++) {
			xfrac = 0;
			for (int x=0; x<GEOPATCH_EDGELEN; x++) {
				vector3d p = GetSpherePoint(xfrac, yfrac);
				double height = geosphere->GetHeight(p);
				*(vts++) = p * (height + 1.0);
				// remember this -- we will need it later
				(col++)->x = height;
				xfrac += GEOPATCH_FRAC;
			}
			yfrac += GEOPATCH_FRAC;
		}
		assert(vts == &vertices[GEOPATCH_NUMVERTICES]);
		// Generate normals & colors for non-edge vertices since they never change
		for (int y=1; y<GEOPATCH_EDGELEN-1; y++) {
			for (int x=1; x<GEOPATCH_EDGELEN-1; x++) {
				// normal
				vector3d x1 = vertices[x-1 + y*GEOPATCH_EDGELEN];
				vector3d x2 = vertices[x+1 + y*GEOPATCH_EDGELEN];
				vector3d y1 = vertices[x + (y-1)*GEOPATCH_EDGELEN];
				vector3d y2 = vertices[x + (y+1)*GEOPATCH_EDGELEN];

				vector3d n = (x2-x1).Cross(y2-y1);
				normals[x + y*GEOPATCH_EDGELEN] = n.Normalized();
				// color
				vector3d p = GetSpherePoint(x*GEOPATCH_FRAC, y*GEOPATCH_FRAC);
				vector3d &col_r = colors[x + y*GEOPATCH_EDGELEN];
				const double height = col_r.x;
				const vector3d &norm = normals[x + y*GEOPATCH_EDGELEN];
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
			for (int x=0; x<GEOPATCH_EDGELEN; x++) {
				vector3d p = GetSpherePoint(x * GEOPATCH_FRAC, 0);
				double height = geosphere->GetHeight(p);
				vertices[x] = p * (height + 1.0);
				// XXX These bounds checks in each edge case are
				// only necessary while the "All these 'if's"
				// comment in FixCOrnerNormalsByEdge stands
				if ((x>0) && (x<GEOPATCH_EDGELEN-1)) {
					colors[x].x = height;
				}
			}
		} else if (edge == 1) {
			for (int y=0; y<GEOPATCH_EDGELEN; y++) {
				vector3d p = GetSpherePoint(1.0, y * GEOPATCH_FRAC);
				double height = geosphere->GetHeight(p);
				int pos = (GEOPATCH_EDGELEN-1) + y*GEOPATCH_EDGELEN;
				vertices[pos] = p * (height + 1.0);
				if ((y>0) && (y<GEOPATCH_EDGELEN-1)) {
					colors[pos].x = height;
				}
			}
		} else if (edge == 2) {
			for (int x=0; x<GEOPATCH_EDGELEN; x++) {
				vector3d p = GetSpherePoint(x * GEOPATCH_FRAC, 1.0);
				double height = geosphere->GetHeight(p);
				int pos = x + (GEOPATCH_EDGELEN-1)*GEOPATCH_EDGELEN;
				vertices[pos] = p * (height + 1.0);
				if ((x>0) && (x<GEOPATCH_EDGELEN-1)) {
					colors[pos].x = height;
				}
			}
		} else {
			for (int y=0; y<GEOPATCH_EDGELEN; y++) {
				vector3d p = GetSpherePoint(0, y * GEOPATCH_FRAC);
				double height = geosphere->GetHeight(p);
				int pos = y * GEOPATCH_EDGELEN;
				vertices[pos] = p * (height + 1.0);
				if ((y>0) && (y<GEOPATCH_EDGELEN-1)) {
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
	
	void Render(vector3d &campos, Plane planes[6]) {
		PiVerify(SDL_mutexP(m_kidsLock)==0);
		if (kids[0]) {
			for (int i=0; i<4; i++) kids[i]->Render(campos, planes);
			SDL_mutexV(m_kidsLock);
		} else {
			SDL_mutexV(m_kidsLock);
			_UpdateVBOs();
			/* frustum test! */
			for (int i=0; i<6; i++) {
				if (planes[i].DistanceToPoint(clipCentroid) <= -clipRadius) {
					return;
				}
			}

			vector3d relpos = clipCentroid - campos;
			glPushMatrix();
			glTranslated(relpos.x, relpos.y, relpos.z);

			Pi::statSceneTris += 2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);

			glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
			glVertexPointer(3, GL_FLOAT, sizeof(VBOVertex), 0);
			glNormalPointer(GL_FLOAT, sizeof(VBOVertex), reinterpret_cast<void *>(3*sizeof(float)));
			glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(VBOVertex), reinterpret_cast<void *>(6*sizeof(float)));
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
			glDrawRangeElements(GL_TRIANGLES, 0, GEOPATCH_NUMVERTICES-1, VBO_COUNT_MID_IDX, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(IDX_VBO_MAIN_OFFSET));
			for (int i=0; i<4; i++) {
				if (edgeFriend[i]) {
					glDrawRangeElements(GL_TRIANGLES, s_hiMinIdx[i], s_hiMaxIdx[i], VBO_COUNT_HI_EDGE, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(IDX_VBO_HI_OFFSET(i)));
				} else {
					glDrawRangeElements(GL_TRIANGLES, s_loMinIdx[i], s_loMaxIdx[i], VBO_COUNT_LO_EDGE, GL_UNSIGNED_SHORT, reinterpret_cast<void*>(IDX_VBO_LO_OFFSET(i)));
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
				
		vector3d centroid = (v[0]+v[1]+v[2]+v[3]).Normalized();
		centroid = (1.0 + geosphere->GetHeight(centroid)) * centroid;

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
				_kids[0] = new GeoPatch(v[0], v01, cn, v30, m_depth+1);
				_kids[1] = new GeoPatch(v01, v[1], v12, cn, m_depth+1);
				_kids[2] = new GeoPatch(cn, v12, v[2], v23, m_depth+1);
				_kids[3] = new GeoPatch(v30, cn, v23, v[3], m_depth+1);
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
				_kids[0]->geosphere = _kids[1]->geosphere = _kids[2]->geosphere = _kids[3]->geosphere = geosphere;
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

unsigned short *GeoPatch::midIndices = 0;
unsigned short *GeoPatch::loEdgeIndices[4];
unsigned short *GeoPatch::hiEdgeIndices[4];
GLuint GeoPatch::indices_vbo;
VBOVertex *GeoPatch::vbotemp;

static const int geo_sphere_edge_friends[6][4] = {
	{ 3, 4, 1, 2 },
	{ 0, 4, 5, 2 },
	{ 0, 1, 5, 3 },
	{ 0, 2, 5, 4 },
	{ 0, 3, 5, 1 },
	{ 1, 4, 3, 2 }
};

#define PLANET_AMBIENT	0.0
static std::list<GeoSphere*> s_allGeospheres;
SDL_mutex *s_allGeospheresLock;

/* Thread that updates geosphere level of detail thingies */
int GeoSphere::UpdateLODThread(void *data)
{
	for(;;) {
		SDL_mutexP(s_allGeospheresLock);
		for(std::list<GeoSphere*>::iterator i = s_allGeospheres.begin();
				i != s_allGeospheres.end(); ++i) {
			if ((*i)->m_runUpdateThread) (*i)->_UpdateLODs();
		}
		SDL_mutexV(s_allGeospheresLock);

		SDL_Delay(10);
	}

	RETURN_ZERO_NONGNU_ONLY;
}

void GeoSphere::_UpdateLODs()
{
	for (int i=0; i<6; i++) {
		m_patches[i]->LODUpdate(m_tempCampos);
	}
	m_runUpdateThread = 0;
}

/* This is to stop threads keeping on iterating over the s_allGeospheres list,
 * which may have been destroyed by exit() (does on lunix anyway...)
 */
static void _LockoutThreadsBeforeExit()
{
	SDL_mutexP(s_allGeospheresLock);
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
	s_allGeospheresLock = SDL_CreateMutex();
	OnChangeDetailLevel();
#ifdef GEOSPHERE_USE_THREADING
	SDL_CreateThread(&GeoSphere::UpdateLODThread, 0);
#endif /* GEOSPHERE_USE_THREADING */
	atexit(&_LockoutThreadsBeforeExit);
}

void GeoSphere::OnChangeDetailLevel()
{
	SDL_mutexP(s_allGeospheresLock);
	for(std::list<GeoSphere*>::iterator i = s_allGeospheres.begin();
			i != s_allGeospheres.end(); ++i) {
		for (int p=0; p<6; p++) if ((*i)->m_patches[p]) delete (*i)->m_patches[p];
	}
	switch (Pi::detail.planets) {
		case 0: GEOPATCH_EDGELEN = 7; break;
		case 1: GEOPATCH_EDGELEN = 15; break;
		case 2: GEOPATCH_EDGELEN = 25; break;
		case 3: GEOPATCH_EDGELEN = 35; break;
		default:
		case 4: GEOPATCH_EDGELEN = 55; break;
	}
	assert(GEOPATCH_EDGELEN <= GEOPATCH_MAX_EDGELEN);
	GeoPatch::Init();
	for(std::list<GeoSphere*>::iterator i = s_allGeospheres.begin();
			i != s_allGeospheres.end(); ++i) {
		(*i)->BuildFirstPatches();
	}
	SDL_mutexV(s_allGeospheresLock);
}

#define GEOSPHERE_TYPE	(m_sbody->type)

GeoSphere::GeoSphere(const SBody *body): m_style(body)
{
	m_vbosToDestroyLock = SDL_CreateMutex();
	m_runUpdateThread = 0;
	m_sbody = body;
	memset(m_patches, 0, 6*sizeof(GeoPatch*));

	SDL_mutexP(s_allGeospheresLock);
	s_allGeospheres.push_back(this);
	SDL_mutexV(s_allGeospheresLock);
}

GeoSphere::~GeoSphere()
{
	SDL_mutexP(s_allGeospheresLock);
	s_allGeospheres.remove(this);
	SDL_mutexV(s_allGeospheresLock);

	for (int i=0; i<6; i++) if (m_patches[i]) delete m_patches[i];
	DestroyVBOs();
	SDL_DestroyMutex(m_vbosToDestroyLock);
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

	m_patches[0] = new GeoPatch(p1, p2, p3, p4, 0);
	m_patches[1] = new GeoPatch(p4, p3, p7, p8, 0);
	m_patches[2] = new GeoPatch(p1, p4, p8, p5, 0);
	m_patches[3] = new GeoPatch(p2, p1, p5, p6, 0);
	m_patches[4] = new GeoPatch(p3, p2, p6, p7, 0);
	m_patches[5] = new GeoPatch(p8, p7, p6, p5, 0);
	for (int i=0; i<6; i++) {
		m_patches[i]->geosphere = this;
		for (int j=0; j<4; j++) {
			m_patches[i]->edgeFriend[j] = m_patches[geo_sphere_edge_friends[i][j]];
		}
	}
	for (int i=0; i<6; i++) m_patches[i]->GenerateMesh();
	for (int i=0; i<6; i++) m_patches[i]->GenerateEdgeNormalsAndColors();
	for (int i=0; i<6; i++) m_patches[i]->UpdateVBOs();
}

static const float g_ambient[4] = { 0, 0, 0, 1.0 };

static void DrawAtmosphereSurface(const vector3d &campos, float rad)
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
	glBegin(GL_TRIANGLE_FAN);
	glVertex3f(0.0f, 1.0f, 0.0f);
	for (int i=0; i<=LONG_SEGS; i++) {
		glVertex3f(sin(latDiff)*sinCosTable[i][0], cos(latDiff), -sin(latDiff)*sinCosTable[i][1]);
	}
	glEnd();

	/* and wound latitudinal strips */
	double lat = latDiff;
	for (int j=1; j<LAT_SEGS; j++, lat += latDiff) {
		glBegin(GL_TRIANGLE_STRIP);
		float cosLat = cos(lat);
		float sinLat = sin(lat);
		float cosLat2 = cos(lat+latDiff);
		float sinLat2 = sin(lat+latDiff);
		for (int i=0; i<=LONG_SEGS; i++) {
			glVertex3f(sinLat*sinCosTable[i][0], cosLat, -sinLat*sinCosTable[i][1]);
			glVertex3f(sinLat2*sinCosTable[i][0], cosLat2, -sinLat2*sinCosTable[i][1]);
		}
		glEnd();
	}

	glPopMatrix();
}

void GeoSphere::Render(vector3d campos, const float radius, const float scale) {
	Plane planes[6];
	glPushMatrix();
	glTranslated(-campos.x, -campos.y, -campos.z);
	GetFrustum(planes);
	const float atmosRadius = ATMOSPHERE_RADIUS;
	
	// no frustum test of entire geosphere, since Space::Render does this
	// for each body using its GetBoundingRadius() value

	if (Render::AreShadersEnabled()) {
		Color atmosCol;
		double atmosDensity;
		matrix4x4d modelMatrix;
		glGetDoublev (GL_MODELVIEW_MATRIX, &modelMatrix[0]);
		vector3d center = modelMatrix * vector3d(0.0, 0.0, 0.0);
		
		GetAtmosphereFlavor(&atmosCol, &atmosDensity);
		atmosDensity *= 0.00005;

		if (atmosDensity > 0.0) {
			GeosphereShader *shader = s_geosphereSkyShader[Render::State::GetNumLights()-1];
			Render::State::UseProgram(shader);
			shader->set_geosphereScale(scale);
			shader->set_geosphereAtmosTopRad(atmosRadius*radius/scale);
			shader->set_geosphereAtmosFogDensity(atmosDensity);
			shader->set_atmosColor(atmosCol.r, atmosCol.g, atmosCol.b, atmosCol.a);
			shader->set_geosphereCenter(center.x, center.y, center.z);
			
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
			// make atmosphere sphere slightly bigger than required so
			// that the edges of the pixel shader atmosphere jizz doesn't
			// show ugly polygonal angles
			DrawAtmosphereSurface(campos, atmosRadius*1.01);
			glDisable(GL_BLEND);
		}

		GeosphereShader *shader = s_geosphereSurfaceShader[Render::State::GetNumLights()-1];
		Render::State::UseProgram(shader);
		shader->set_geosphereScale(scale);
		shader->set_geosphereAtmosTopRad(atmosRadius*radius/scale);
		shader->set_geosphereAtmosFogDensity(atmosDensity);
		shader->set_atmosColor(atmosCol.r, atmosCol.g, atmosCol.b, atmosCol.a);
		shader->set_geosphereCenter(center.x, center.y, center.z);
	}
	glPopMatrix();

	if (!m_patches[0]) BuildFirstPatches();

	const float black[4] = { 0,0,0,0 };
	float ambient[4];// = { 0.1, 0.1, 0.1, 1.0 };

	// save old global ambient
	float oldAmbient[4];
	glGetFloatv(GL_LIGHT_MODEL_AMBIENT, oldAmbient);

	// give planet some ambient lighting if the viewer is close to it
	{
		double camdist = campos.Length();
		camdist = 0.1 / (camdist*camdist);
		// why the fuck is this returning 0.1 when we are sat on the planet??
		// JJ: Because campos is relative to a unit-radius planet - 1.0 at the surface
		// XXX oh well, it is the value we want anyway...
		ambient[0] = ambient[1] = ambient[2] = float(camdist);
		ambient[3] = 1.0f;
	}
	
	glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv (GL_FRONT, GL_SPECULAR, black);
	glMaterialfv (GL_FRONT, GL_EMISSION, black);
	glEnable(GL_COLOR_MATERIAL);

//	glLineWidth(1.0);
//	glPolygonMode(GL_FRONT, GL_LINE);
	for (int i=0; i<6; i++) {
		m_patches[i]->Render(campos, planes);
	}
	Render::State::UseProgram(0);

	glDisable(GL_COLOR_MATERIAL);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, oldAmbient);

	// if the update thread has deleted any geopatches, destroy the vbos
	// associated with them
	DestroyVBOs();
		/*this->m_tempCampos = campos;
		UpdateLODThread(this);
		return;*/
	
	if (!m_runUpdateThread) {
		this->m_tempCampos = campos;
		m_runUpdateThread = 1;
	}
#ifndef GEOSPHERE_USE_THREADING
	m_tempCampos = campos;
	_UpdateLODs();
#endif /* !GEOSPHERE_USE_THREADING */
}

