#include "libs.h"
#include "GeoSphere.h"
#include "perlin.h"
#include "Pi.h"
#include "StarSystem.h"

// tri edge lengths
#define GEOPATCH_SUBDIVIDE_AT_CAMDIST	2.2
#define GEOPATCH_MAX_DEPTH	14
// must be power of two + 1
#define GEOPATCH_EDGELEN	33
#define GEOPATCH_NUMVERTICES	(GEOPATCH_EDGELEN*GEOPATCH_EDGELEN)

static const double GEOPATCH_FRAC = 1.0 / (double)(GEOPATCH_EDGELEN-1);

#define PRINT_VECTOR(_v) printf("%f,%f,%f\n", (_v).x, (_v).y, (_v).z);

#define USE_VBO	GLEW_ARB_vertex_buffer_object
//#define USE_VBO 0

#pragma pack(4)
struct VBOVertex
{
	float x,y,z;
	float nx,ny,nz;
	float cr,cg,cb;
};
#pragma pack()

class GeoPatch {
public:
	vector3d v[4];
	vector3d *vertices;
	vector3d *normals;
	vector3d *colors;
	GLuint m_vbo;
	static unsigned short *indices;
	static unsigned short *loEdgeIndices[4];
	static GLuint indices_vbo[5];
	static VBOVertex vbotemp[GEOPATCH_NUMVERTICES];
	GeoPatch *kids[4];
	GeoPatch *parent;
	GeoPatch *edgeFriend[4]; // [0]=v01, [1]=v12, [2]=v20
	GeoSphere *geosphere;
	double m_roughLength;
	int m_depth;
	GeoPatch() {
		memset(this, 0, sizeof(GeoPatch));
	}
	GeoPatch(vector3d v0, vector3d v1, vector3d v2, vector3d v3, int depth) {
		memset(this, 0, sizeof(GeoPatch));
		v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3;
		m_depth = depth;
		m_roughLength = GEOPATCH_SUBDIVIDE_AT_CAMDIST / pow(2.0, depth);
		if (USE_VBO) glGenBuffersARB(1, &m_vbo);
		if (!indices) {
			indices = new unsigned short[2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1)*3];
			const int numLoEdgeTris = GEOPATCH_EDGELEN/2;
			for (int i=0; i<4; i++) loEdgeIndices[i] = new unsigned short[3*numLoEdgeTris];
			unsigned short *idx = indices;
			for (int x=0; x<GEOPATCH_EDGELEN-1; x++) {
				for (int y=0; y<GEOPATCH_EDGELEN-1; y++) {
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
			// these edge indices are for patches with no
			// neighbour of equal or greater detail -- they reduce
			// their edge complexity by 1 division
			idx = loEdgeIndices[0];
			for (int x=0; x<GEOPATCH_EDGELEN-1; x+=2) {
				idx[0] = x;
				idx[1] = x+2;
				idx[2] = x+1+GEOPATCH_EDGELEN;
				idx += 3;
			}
			idx = loEdgeIndices[1];
			for (int y=0; y<GEOPATCH_EDGELEN-1; y+=2) {
				idx[0] = (GEOPATCH_EDGELEN-1) + y*GEOPATCH_EDGELEN;
				idx[1] = (GEOPATCH_EDGELEN-1) + (y+2)*GEOPATCH_EDGELEN;
				idx[2] = (GEOPATCH_EDGELEN-2) + (y+1)*GEOPATCH_EDGELEN;
				idx += 3;
			}
			idx = loEdgeIndices[2];
			for (int x=0; x<GEOPATCH_EDGELEN-1; x+=2) {
				idx[0] = x+GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-1);
				idx[2] = x+2+GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-1);
				idx[1] = x+1+GEOPATCH_EDGELEN*(GEOPATCH_EDGELEN-2);
				idx += 3;
			}
			idx = loEdgeIndices[3];
			for (int y=0; y<GEOPATCH_EDGELEN-1; y+=2) {
				idx[0] = y*GEOPATCH_EDGELEN;
				idx[2] = (y+2)*GEOPATCH_EDGELEN;
				idx[1] = 1 + (y+1)*GEOPATCH_EDGELEN;
				idx += 3;
			}

			if (USE_VBO) {
				glGenBuffersARB(5, indices_vbo);
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_vbo[0]);
				glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1)*3,
						indices, GL_STATIC_DRAW);
				for (int i=0; i<4; i++) {
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_vbo[i+1]);
					glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*3*(GEOPATCH_EDGELEN/2),
						loEdgeIndices[i], GL_STATIC_DRAW);
				}
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
			}
		}
	}
	~GeoPatch() {
		for (int i=0; i<4; i++) {
			if (edgeFriend[i]) edgeFriend[i]->NotifyEdgeFriendDeleted(this);
		}
		for (int i=0; i<4; i++) if (kids[i]) delete kids[i];
		if (vertices) delete vertices;
		if (normals) delete normals;
		if (colors) delete colors;
		if (USE_VBO) glDeleteBuffersARB(1, &m_vbo);
	}
	void UpdateVBOs() {
		if (USE_VBO) {
			for (int i=0; i<GEOPATCH_NUMVERTICES; i++)
			{
				VBOVertex *pData = vbotemp + i;
				pData->x = (float)vertices[i].x;
				pData->y = (float)vertices[i].y;
				pData->z = (float)vertices[i].z;
				pData->nx = (float)normals[i].x;
				pData->ny = (float)normals[i].y;
				pData->nz = (float)normals[i].z;
				pData->cr = (float)colors[i].x;
				pData->cg = (float)colors[i].y;
				pData->cb = (float)colors[i].z;
			}
			glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
			glBufferDataARB(GL_ARRAY_BUFFER, sizeof(VBOVertex)*GEOPATCH_NUMVERTICES, vbotemp, GL_STATIC_DRAW);
			glBindBufferARB(GL_ARRAY_BUFFER, 0);
		}
	}	
	/* not quite edge, since we share edge vertices so that would be
	 * fucking pointless. one position inwards. used to make edge normals
	 * for adjacent tiles */
	void GetEdgeMinusOneVerticesFlipped(int edge, vector3d ev[GEOPATCH_EDGELEN]) {
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
	static void GetEdge(vector3d *array, int edge, vector3d ev[GEOPATCH_EDGELEN]) {
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
	static void SetEdge(vector3d *array, int edge, const vector3d ev[GEOPATCH_EDGELEN]) {
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


	void FixEdgeNormals(const int edge, const vector3d ev[GEOPATCH_EDGELEN]) {
		vector3d x1, x2, y1, y2;
		int x, y;
		switch (edge) {
		case 0:
			for (x=1; x<GEOPATCH_EDGELEN-1; x++) {
				vector3d x1 = vertices[x-1];
				vector3d x2 = vertices[x+1];
				vector3d y1 = ev[x];
				vector3d y2 = vertices[x + GEOPATCH_EDGELEN];
				normals[x] = vector3d::Cross(x2-x1, y2-y1).Normalized();
			}
			break;
		case 1:
			x = GEOPATCH_EDGELEN-1;
			for (y=1; y<GEOPATCH_EDGELEN-1; y++) {
				vector3d x1 = vertices[(x-1) + y*GEOPATCH_EDGELEN];
				vector3d x2 = ev[y];
				vector3d y1 = vertices[x + (y-1)*GEOPATCH_EDGELEN];
				vector3d y2 = vertices[x + (y+1)*GEOPATCH_EDGELEN];
				normals[x + y*GEOPATCH_EDGELEN] = vector3d::Cross(x2-x1, y2-y1).Normalized();
			}
			break;
		case 2:
			y = GEOPATCH_EDGELEN-1;
			for (x=1; x<GEOPATCH_EDGELEN-1; x++) {
				vector3d x1 = vertices[x-1 + y*GEOPATCH_EDGELEN];
				vector3d x2 = vertices[x+1 + y*GEOPATCH_EDGELEN];
				vector3d y1 = vertices[x + (y-1)*GEOPATCH_EDGELEN];
				vector3d y2 = ev[GEOPATCH_EDGELEN-1-x];
				normals[x + y*GEOPATCH_EDGELEN] = vector3d::Cross(x2-x1, y2-y1).Normalized();
			}
			break;
		case 3:
			for (y=1; y<GEOPATCH_EDGELEN-1; y++) {
				vector3d x1 = ev[GEOPATCH_EDGELEN-1-y];
				vector3d x2 = vertices[1 + y*GEOPATCH_EDGELEN];
				vector3d y1 = vertices[(y-1)*GEOPATCH_EDGELEN];
				vector3d y2 = vertices[(y+1)*GEOPATCH_EDGELEN];
				normals[y*GEOPATCH_EDGELEN] = vector3d::Cross(x2-x1, y2-y1).Normalized();
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
		vector3d ev[GEOPATCH_EDGELEN];
		vector3d en[GEOPATCH_EDGELEN];
		vector3d ec[GEOPATCH_EDGELEN];
		vector3d ev2[GEOPATCH_EDGELEN];
		vector3d en2[GEOPATCH_EDGELEN];
		vector3d ec2[GEOPATCH_EDGELEN];
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
	void MakeCornerNormal(vector3d ev[GEOPATCH_EDGELEN], vector3d ev2[GEOPATCH_EDGELEN]) {
		int p;
		vector3d x1,x2,y1,y2;
		switch (corner) {
		case 0:
			x1 = ev[GEOPATCH_EDGELEN-1];
			x2 = vertices[1];
			y1 = ev2[0];
			y2 = vertices[GEOPATCH_EDGELEN];
			normals[0] = vector3d::Cross(x2-x1, y2-y1).Normalized();
			break;
		case 1:
			p = GEOPATCH_EDGELEN-1;
			x1 = vertices[p-1];
			x2 = ev2[0];
			y1 = ev[GEOPATCH_EDGELEN-1];
			y2 = vertices[p + GEOPATCH_EDGELEN];
			normals[p] = vector3d::Cross(x2-x1, y2-y1).Normalized();
			break;
		case 2:
			p = GEOPATCH_EDGELEN-1;
			x1 = vertices[(p-1) + p*GEOPATCH_EDGELEN];
			x2 = ev[GEOPATCH_EDGELEN-1];
			y1 = vertices[p + (p-1)*GEOPATCH_EDGELEN];
			y2 = ev2[0];
			normals[p + p*GEOPATCH_EDGELEN] = vector3d::Cross(x2-x1, y2-y1).Normalized();
			break;
		case 3:
			p = GEOPATCH_EDGELEN-1;
			x1 = ev2[0];
			x2 = vertices[1 + p*GEOPATCH_EDGELEN];
			y1 = vertices[(p-1)*GEOPATCH_EDGELEN];
			y2 = ev[GEOPATCH_EDGELEN-1];
			normals[p*GEOPATCH_EDGELEN] = vector3d::Cross(x2-x1, y2-y1).Normalized();
			break;
		}
	}

	void FixCornerNormalsByEdge(int edge, vector3d ev[GEOPATCH_EDGELEN]) {
		vector3d ev2[GEOPATCH_EDGELEN];
		vector3d x1, x2, y1, y2;
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

	void GenerateNormals() {
		if (normals) return;

		normals = new vector3d[GEOPATCH_NUMVERTICES];
		
		for (int y=1; y<GEOPATCH_EDGELEN-1; y++) {
			for (int x=1; x<GEOPATCH_EDGELEN-1; x++) {
				vector3d x1 = vertices[x-1 + y*GEOPATCH_EDGELEN];
				vector3d x2 = vertices[x+1 + y*GEOPATCH_EDGELEN];
				vector3d y1 = vertices[x + (y-1)*GEOPATCH_EDGELEN];
				vector3d y2 = vertices[x + (y+1)*GEOPATCH_EDGELEN];

				vector3d n = vector3d::Cross(x2-x1, y2-y1);
				normals[x + y*GEOPATCH_EDGELEN] = n.Normalized();
			}
		}
		vector3d ev[4][GEOPATCH_EDGELEN];
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

		UpdateVBOs();
	}

	/* in patch surface coords, [0,1] */
	vector3d GetSpherePoint(double x, double y) {
		return (v[0] + x*(1.0-y)*(v[1]-v[0]) +
			    x*y*(v[2]-v[0]) +
			    (1.0-x)*y*(v[3]-v[0])).Normalized();
	}

	void GenerateMesh() {
		if (!vertices) {
			vertices = new vector3d[GEOPATCH_NUMVERTICES];
			colors = new vector3d[GEOPATCH_NUMVERTICES];
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
					*(col++) = geosphere->GetColor(p, height);
					xfrac += GEOPATCH_FRAC;
				}
				yfrac += GEOPATCH_FRAC;
			}
			assert(vts == &vertices[GEOPATCH_NUMVERTICES]);
		}
	}
	void OnEdgeFriendChanged(int edge, GeoPatch *e) {
		edgeFriend[edge] = e;
		vector3d ev[GEOPATCH_EDGELEN];
		int we_are = e->GetEdgeIdxOf(this);
		e->GetEdgeMinusOneVerticesFlipped(we_are, ev);
		/* now we have a valid edge, fix the edge vertices */
		if (edge == 0) {
			for (int x=0; x<GEOPATCH_EDGELEN; x++) {
				vector3d p = GetSpherePoint(x * GEOPATCH_FRAC, 0);
				double height = geosphere->GetHeight(p);
				vertices[x] = p * (height + 1.0);
				colors[x] = geosphere->GetColor(p, height);
			}
		} else if (edge == 1) {
			for (int y=0; y<GEOPATCH_EDGELEN; y++) {
				vector3d p = GetSpherePoint(1.0, y * GEOPATCH_FRAC);
				double height = geosphere->GetHeight(p);
				int pos = (GEOPATCH_EDGELEN-1) + y*GEOPATCH_EDGELEN;
				vertices[pos] = p * (height + 1.0);
				colors[pos] = geosphere->GetColor(p, height);
			}
		} else if (edge == 2) {
			for (int x=0; x<GEOPATCH_EDGELEN; x++) {
				vector3d p = GetSpherePoint(x * GEOPATCH_FRAC, 1.0);
				double height = geosphere->GetHeight(p);
				int pos = x + (GEOPATCH_EDGELEN-1)*GEOPATCH_EDGELEN;
				vertices[pos] = p * (height + 1.0);
				colors[pos] = geosphere->GetColor(p, height);
			}
		} else {
			for (int y=0; y<GEOPATCH_EDGELEN; y++) {
				vector3d p = GetSpherePoint(0, y * GEOPATCH_FRAC);
				double height = geosphere->GetHeight(p);
				int pos = y * GEOPATCH_EDGELEN;
				vertices[pos] = p * (height + 1.0);
				colors[pos] = geosphere->GetColor(p, height);
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
			fprintf(stderr, "Bad. not fixing up edge\n");
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
	
	void Render(vector3d &campos) {
		if (kids[0]) {
			for (int i=0; i<4; i++) kids[i]->Render(campos);
		} else {
			Pi::statSceneTris += 2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
			glEnableClientState(GL_COLOR_ARRAY);
			if (USE_VBO) {
				glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
				glVertexPointer(3, GL_FLOAT, sizeof(VBOVertex), 0);
				glNormalPointer(GL_FLOAT, sizeof(VBOVertex), (void *)(3*sizeof(float)));
				glColorPointer(3, GL_FLOAT, sizeof(VBOVertex), (void *)(6*sizeof(float)));
				glEnableClientState(GL_ELEMENT_ARRAY_BUFFER);
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_vbo[0]);
				glDrawElements(GL_TRIANGLES, 2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1)*3, GL_UNSIGNED_SHORT, 0);
				for (int i=0; i<4; i++) {
					if (edgeFriend[i]) continue;
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_vbo[i+1]);
					glDrawElements(GL_TRIANGLES, 3*(GEOPATCH_EDGELEN/2), GL_UNSIGNED_SHORT, 0);
				}
				glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
				glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
				glDisableClientState(GL_ELEMENT_ARRAY_BUFFER);
			} else {
				glVertexPointer(3, GL_DOUBLE, 0, &vertices[0].x);
				glNormalPointer(GL_DOUBLE, 0, &normals[0].x);
				glColorPointer(3, GL_DOUBLE, 0, &colors[0].x);
				glDrawElements(GL_TRIANGLES, 2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1)*3, GL_UNSIGNED_SHORT, indices);
				for (int i=0; i<4; i++) {
					if (edgeFriend[i]) continue;
					// Draw reduced division edge when
					// we have no neighbour (ie neighbour
					// is lower LOD)
					// XXX this is not ideal because we draw
					// more triangles than we need to
					glDrawElements(GL_TRIANGLES, (GEOPATCH_EDGELEN/2)*3, GL_UNSIGNED_SHORT, loEdgeIndices[i]);
				}
			}
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_COLOR_ARRAY);
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
				vector3d v01, v12, v23, v30;
				v01 = (v[0]+v[1]).Normalized();
				v12 = (v[1]+v[2]).Normalized();
				v23 = (v[2]+v[3]).Normalized();
				v30 = (v[3]+v[0]).Normalized();
				kids[0] = new GeoPatch(v[0], v01, centroid, v30, m_depth+1);
				kids[1] = new GeoPatch(v01, v[1], v12, centroid, m_depth+1);
				kids[2] = new GeoPatch(centroid, v12, v[2], v23, m_depth+1);
				kids[3] = new GeoPatch(v30, centroid, v23, v[3], m_depth+1);
				// hm.. edges. Not right to pass this
				// edgeFriend...
				kids[0]->edgeFriend[0] = GetEdgeFriendForKid(0, 0);
				kids[0]->edgeFriend[1] = kids[1];
				kids[0]->edgeFriend[2] = kids[3];
				kids[0]->edgeFriend[3] = GetEdgeFriendForKid(0, 3);
				kids[1]->edgeFriend[0] = GetEdgeFriendForKid(1, 0);
				kids[1]->edgeFriend[1] = GetEdgeFriendForKid(1, 1);
				kids[1]->edgeFriend[2] = kids[2];
				kids[1]->edgeFriend[3] = kids[0];
				kids[2]->edgeFriend[0] = kids[1];
				kids[2]->edgeFriend[1] = GetEdgeFriendForKid(2, 1);
				kids[2]->edgeFriend[2] = GetEdgeFriendForKid(2, 2);
				kids[2]->edgeFriend[3] = kids[3];
				kids[3]->edgeFriend[0] = kids[0];
				kids[3]->edgeFriend[1] = kids[2];
				kids[3]->edgeFriend[2] = GetEdgeFriendForKid(3, 2);
				kids[3]->edgeFriend[3] = GetEdgeFriendForKid(3, 3);
				kids[0]->parent = kids[1]->parent = kids[2]->parent = kids[3]->parent = this;
				kids[0]->geosphere = kids[1]->geosphere = kids[2]->geosphere = kids[3]->geosphere = geosphere;
				for (int i=0; i<4; i++) kids[i]->GenerateMesh();
				for (int i=0; i<4; i++) edgeFriend[i]->NotifyEdgeFriendSplit(this);
				for (int i=0; i<4; i++) kids[i]->GenerateNormals();
			}
			for (int i=0; i<4; i++) kids[i]->LODUpdate(campos);
		} else {
			if (canMerge && kids[0]) {
				for (int i=0; i<4; i++) { delete kids[i]; kids[i] = 0; }
			}
		}
	}
};

unsigned short *GeoPatch::indices = 0;
unsigned short *GeoPatch::loEdgeIndices[4];
GLuint GeoPatch::indices_vbo[5];
VBOVertex GeoPatch::vbotemp[GEOPATCH_NUMVERTICES];


static const int geo_sphere_edge_friends[6][4] = {
	{ 3, 4, 1, 2 },
	{ 0, 4, 5, 2 },
	{ 0, 1, 5, 3 },
	{ 0, 2, 5, 4 },
	{ 0, 3, 5, 1 },
	{ 1, 4, 3, 2 }
};

#define PLANET_AMBIENT	0.1

void GeoSphere::SetColor(const float col[4])
{
	m_ambColor[0] = col[0]*PLANET_AMBIENT;
	m_ambColor[1] = col[1]*PLANET_AMBIENT;
	m_ambColor[2] = col[2]*PLANET_AMBIENT;
	m_ambColor[3] = col[3];
	for (int i=0; i<4; i++) m_diffColor[i] = col[i];
}

/*
 * Define crater radii in angular size on surface of sphere (in radians)
 */
void GeoSphere::AddCraters(MTRand &rand, int num, double minAng, double maxAng)
{
	assert(m_craters == 0);
	m_numCraters = num;
	m_craters = new crater_t[num];
	minAng = cos(minAng);
	maxAng = cos(maxAng);
	for (int i=0; i<num; i++) {
		m_craters[i].pos = vector3d(
				rand.Double(-1.0, 1.0),
				rand.Double(-1.0, 1.0),
				rand.Double(-1.0, 1.0)).Normalized();
		m_craters[i].size = rand.Double(minAng, maxAng);
	}
}

#define GEOSPHERE_SEED	(m_sbody->seed)
#define GEOSPHERE_TYPE	(m_sbody->type)
//#define GEOSPHERE_TYPE SBody::TYPE_PLANET_INDIGENOUS_LIFE

GeoSphere::GeoSphere(const SBody *body)
{
	MTRand rand;
	rand.seed(body->seed);
	m_sbody = body;
	m_numCraters = 0;
	m_craters = 0;
	m_maxHeight = 0.0014/sqrt(m_sbody->radius.ToDouble());
	m_invMaxHeight = 1.0 / m_maxHeight;
//	printf("%s max mountain height: %f meters\n",m_sbody->name.c_str(), m_maxHeight * m_sbody->GetRadius());

	for (int i=0; i<16; i++) m_crap[i] = rand.Double();
	m_sealevel = rand.Double();
	m_fractalOffset = vector3d(rand.Double(255), rand.Double(255), rand.Double(255));
	
	switch (GEOSPHERE_TYPE) {
		case SBody::TYPE_PLANET_DWARF:
			AddCraters(rand, rand.Int32(10,30), M_PI*0.005, M_PI*0.05);
			break;
		default: break;
	}

	memset(m_patches, 0, sizeof(m_patches));
	m_patches[0] = 0;

	float col[4] = {1,1,1,1};
	SetColor(col);
}

GeoSphere::~GeoSphere()
{
	for (int i=0; i<6; i++) if (m_patches[i]) delete m_patches[i];
	delete [] m_craters;
}

static const float g_ambient[4] = { .1, .1, .1, 1.0 };

void GeoSphere::Render(vector3d campos) {
	if (m_patches[0] == 0) {
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
		for (int i=0; i<6; i++) m_patches[i]->GenerateNormals();
	}
	for (int i=0; i<6; i++) {
		m_patches[i]->LODUpdate(campos);
	}
	glLightModelfv (GL_LIGHT_MODEL_AMBIENT, g_ambient);
	glMaterialfv (GL_FRONT, GL_AMBIENT, m_ambColor);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, m_diffColor);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	for (int i=0; i<6; i++) {
		m_patches[i]->Render(campos);
	}
	glDisable(GL_COLOR_MATERIAL);
}

inline double octavenoise(int octaves, double div, vector3d p)
{
	double n = 0;
	double wank = 1.0;
	double jizm = 1.0;
	while (octaves--) {
		n += wank * noise(jizm*p);
		wank *= div;
		jizm *= 2.0;
	}
	return n;
}

/*
 * p should be a normalised turd on surface of planet.
 */
double GeoSphere::GetHeight(vector3d p)
{
	double n;
	double div;
	p += m_fractalOffset;
	// changing the input point to noise function has interesting results
//	switch (GEOSPHERE_SEED & 1) {
//		case 0: pmul = 1.0 + (double)((GEOSPHERE_SEED>>8)&31); break;
//		// sworly patterns
//		case 1: pmul = noise(2*p.x, p.y*20, 2*p.z); break;
//	}
	switch (GEOSPHERE_TYPE) {
		case SBody::TYPE_PLANET_SMALL_GAS_GIANT:
		case SBody::TYPE_PLANET_MEDIUM_GAS_GIANT:
		case SBody::TYPE_PLANET_LARGE_GAS_GIANT:
		case SBody::TYPE_PLANET_VERY_LARGE_GAS_GIANT:
			return 0;
		case SBody::TYPE_PLANET_ASTEROID:
		case SBody::TYPE_PLANET_LARGE_ASTEROID:
			return 0.1*octavenoise(10, 0.45+0.1*noise(2*p), p);
		case SBody::TYPE_PLANET_DWARF:
		case SBody::TYPE_PLANET_SMALL:
		case SBody::TYPE_PLANET_CO2:
		case SBody::TYPE_PLANET_METHANE:
		case SBody::TYPE_PLANET_WATER_THICK_ATMOS:
		case SBody::TYPE_PLANET_CO2_THICK_ATMOS:
		case SBody::TYPE_PLANET_METHANE_THICK_ATMOS:
		case SBody::TYPE_PLANET_HIGHLY_VOLCANIC:
			switch (GEOSPHERE_SEED & 1) {
				case 0:	div = 0.52 + 0.18*m_crap[1]; break;
				default: div = 0.5 + 0.18*octavenoise(4, 0.5, 256*m_crap[1]*p); break;
			}
			// continent types
			switch ((GEOSPHERE_SEED>>4) & 3) {
				case 0:	n = 1.0 - fabs(octavenoise(18, div, p)); break;
				case 1: n = fabs(octavenoise(18, div, p)); break;
				default: n = 0.5*(1.0+octavenoise(18, div, p)); break;
			}

			// weird loopy blobs
			//n += 0.5*0.3*(1.0+octavenoise(8, 0.5f + 0.25f*noise(2.0*p), p+vector3d(0,0,noise(32*p))));
			//n += 0.5*0.3*octavenoise(8, 0.5, 256.0*p);
			
			n *= m_maxHeight;
			break;
	   	case SBody::TYPE_PLANET_INDIGENOUS_LIFE:
			div = 0.5 + 0.18*octavenoise(4, 0.5, 256*p);
			n = 0.5*(1.0+octavenoise(18, div, p));
			n += 0.1*(1.0 - fabs(octavenoise(18, 0.5, 256.0*p)));
			return (n<m_sealevel ? 0.0 : m_maxHeight*(n-m_sealevel));
		default:
			return 0;
	}
//	n = octavenoise(12, 0.5, pmul*p);
/*	switch (m_sbody->seed & 3) {
		case 0: n = 1.0 - fabs(n); break;
		case 1: n = fabs(n); break;
		default: break;
	}*/
	double crater = 0;
	for (int i=0; i<m_numCraters; i++) {
		const double dot = vector3d::Dot(m_craters[i].pos, p);
		const double depth = 2.0*(m_craters[i].size - 1.0);
		if (dot > m_craters[i].size) crater += depth * MIN(-(m_craters[i].size-dot)/(1.0-m_craters[i].size),
							0.5); // <-- proportion of crater to be slope, before flat bottom
	}
	return n + crater;
}

static inline vector3d interpolate_color(double n, vector3d start, vector3d end)
{
	n = CLAMP(n, 0.0f, 1.0f);
	return start + (end-start)*n;
}

inline vector3d GeoSphere::GetColor(vector3d &p, double height)
{
	double n;
	switch (GEOSPHERE_TYPE) {
		case SBody::TYPE_PLANET_SMALL_GAS_GIANT:
		case SBody::TYPE_PLANET_MEDIUM_GAS_GIANT:
		case SBody::TYPE_PLANET_LARGE_GAS_GIANT:
		case SBody::TYPE_PLANET_VERY_LARGE_GAS_GIANT:
			n = octavenoise(12, 0.5f*m_crap[0] + 0.25f, noise(vector3d(p.x, p.y*m_sbody->radius.ToDouble(), p.z))*p);
			n = (1.0f + n)*0.5f;
			switch (GEOSPHERE_SEED & 3) {
				// jupiterish
				case 0: return interpolate_color(n, vector3d(.50,.22,.18), vector3d(.99,.76,.62));
				// saturnish
				case 1:	return interpolate_color(n, vector3d(.69, .53, .43), vector3d(.99, .76, .62));
				// neptuny
				case 2:	return interpolate_color(n*n, vector3d(.21, .34, .54), vector3d(.31, .44, .73)); 
				// uranussy
				default: return interpolate_color(n, vector3d(.63, .76, .77), vector3d(.70,.85,.86));
			}
		case SBody::TYPE_PLANET_ASTEROID:
		case SBody::TYPE_PLANET_LARGE_ASTEROID:
			return vector3d(.4,.4,.4);
		case SBody::TYPE_PLANET_DWARF:
		case SBody::TYPE_PLANET_SMALL:
		case SBody::TYPE_PLANET_CO2:
		case SBody::TYPE_PLANET_METHANE:
		case SBody::TYPE_PLANET_WATER_THICK_ATMOS:
		case SBody::TYPE_PLANET_CO2_THICK_ATMOS:
		case SBody::TYPE_PLANET_METHANE_THICK_ATMOS:
		case SBody::TYPE_PLANET_HIGHLY_VOLCANIC:
			n = m_invMaxHeight*height - 0.5;
			return interpolate_color(n, vector3d(.2,.2,.2), vector3d(.6,.6,.6));
	   	case SBody::TYPE_PLANET_INDIGENOUS_LIFE:
			n = m_invMaxHeight*height;
			if (n == 0) return vector3d(0,0,0.5);
			else return interpolate_color(n, vector3d(0,.8,0), vector3d(.3,.6,0));
		default:
			return vector3d(1,0,1);

	}
	return vector3d(m_invMaxHeight*height, m_invMaxHeight*height,1);
}
