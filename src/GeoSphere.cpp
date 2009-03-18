#include "libs.h"
#include "GeoSphere.h"
#include "perlin.h"
#include "Pi.h"

//#define USE_VBO
// tri edge lengths
#define GEOPATCH_SUBDIVIDE_AT_CAMDIST	1.2
#define GEOPATCH_MAX_DEPTH	16
// must be power of two + 1
#define GEOPATCH_EDGELEN	17
#define GEOPATCH_NUMVERTICES	(GEOPATCH_EDGELEN*GEOPATCH_EDGELEN)

#define PRINT_VECTOR(_v) printf("%f,%f,%f\n", (_v).x, (_v).y, (_v).z);

class GeoPatch {
public:
	vector3d v[4];
	vector3d *vertices;
	vector3d *normals;
	vector3f *colors;
	GLuint m_vbo[2];
	static unsigned short *indices;
	static GLuint indices_vbo;
	GeoPatch *kids[4];
	GeoPatch *parent;
	GeoPatch *edgeFriend[4]; // [0]=v01, [1]=v12, [2]=v20
	GeoSphere *geosphere;
	double m_roughLength;
	int m_depth;
	GeoPatch() {
		memset(this, 0, sizeof(GeoPatch));
	}
	GeoPatch(vector3d v0, vector3d v1, vector3d v2, vector3d v3) {
		memset(this, 0, sizeof(GeoPatch));
		v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3;
		m_roughLength = MAX((v0-v2).Length(), (v1-v3).Length());
#ifdef USE_VBO
		glGenBuffersARB(2, m_vbo);
#endif /* USE_VBO */
		if (!indices) {
			indices = new unsigned short[2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1)*3];
			unsigned short *idx = indices;
			int wank=0;
			for (int x=0; x<GEOPATCH_EDGELEN-1; x++) {
				for (int y=0; y<GEOPATCH_EDGELEN-1; y++) {
					idx[0] = x + GEOPATCH_EDGELEN*y;
					idx[1] = x+1 + GEOPATCH_EDGELEN*y;
					idx[2] = x + GEOPATCH_EDGELEN*(y+1);
					idx+=3;
					wank++;

					idx[0] = x+1 + GEOPATCH_EDGELEN*y;
					idx[1] = x+1 + GEOPATCH_EDGELEN*(y+1);
					idx[2] = x + GEOPATCH_EDGELEN*(y+1);
					idx+=3;
					wank++;
				}
			}
			assert(wank == 2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1));
#ifdef USE_VBO
			glGenBuffersARB(1, &indices_vbo);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1)*3,
					indices, GL_STATIC_DRAW);
#endif /* USE_VBO */
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
#ifdef USE_VBO
		if (m_vbo[0]) glDeleteBuffersARB(2, m_vbo);
#endif /* USE_VBO */
	}
	void UpdateVBOs() {
#ifdef USE_VBO
		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo[0]);
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(double)*3*GEOPATCH_EDGELEN*GEOPATCH_EDGELEN, vertices, GL_STATIC_DRAW);
		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo[1]);
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(double)*3*GEOPATCH_EDGELEN*GEOPATCH_EDGELEN, normals, GL_STATIC_DRAW);
#endif /* USE_VBO */
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
		//assert(parent->edgeFriend[edge]);
		//GeoPatch *e = parent->edgeFriend[edge];
		//int we_are = e->GetEdgeIdxOf(parent);
		vector3d ev[GEOPATCH_EDGELEN];
		vector3d en[GEOPATCH_EDGELEN];
		vector3d ev2[GEOPATCH_EDGELEN];
		vector3d en2[GEOPATCH_EDGELEN];
		GetEdge(parent->vertices, edge, ev);
		GetEdge(parent->normals, edge, en);

		int kid_idx = parent->GetChildIdx(this);
		if (edge == kid_idx) {
			// use first half of edge
			for (int i=0; i<=GEOPATCH_EDGELEN/2; i++) {
				ev2[i<<1] = ev[i];
				en2[i<<1] = en[i];
			}
		} else {
			// use 2nd half of edge
			for (int i=GEOPATCH_EDGELEN/2; i<GEOPATCH_EDGELEN; i++) {
				ev2[(i-(GEOPATCH_EDGELEN/2))<<1] = ev[i];
				en2[(i-(GEOPATCH_EDGELEN/2))<<1] = en[i];
			}
		}
		// interpolate!!
		for (int i=1; i<GEOPATCH_EDGELEN; i+=2) {
			ev2[i] = (ev2[i-1]+ev2[i+1]) * 0.5;
			en2[i] = (en2[i-1]+en2[i+1]).Normalized();
		}
		SetEdge(this->vertices, edge, ev2);
		SetEdge(this->normals, edge, en2);
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

		// corners
		{
			vector3d x1 = ev[3][GEOPATCH_EDGELEN-1];
			vector3d x2 = vertices[1];
			vector3d y1 = ev[0][0];
			vector3d y2 = vertices[GEOPATCH_EDGELEN];
			normals[0] = vector3d::Cross(x2-x1, y2-y1).Normalized();
		}	
		{
			const int x = GEOPATCH_EDGELEN-1;
			vector3d x1 = vertices[x-1];
			vector3d x2 = ev[1][0];
			vector3d y1 = ev[0][GEOPATCH_EDGELEN-1];
			vector3d y2 = vertices[x + GEOPATCH_EDGELEN];
			normals[x] = vector3d::Cross(x2-x1, y2-y1).Normalized();
		}
		{
			const int p = GEOPATCH_EDGELEN-1;
			vector3d x1 = vertices[(p-1) + p*GEOPATCH_EDGELEN];
			vector3d x2 = ev[1][GEOPATCH_EDGELEN-1];
			vector3d y1 = vertices[p + (p-1)*GEOPATCH_EDGELEN];
			vector3d y2 = ev[2][0];
			normals[p + p*GEOPATCH_EDGELEN] = vector3d::Cross(x2-x1, y2-y1).Normalized();
		}
		{
			const int y = GEOPATCH_EDGELEN-1;
			vector3d x1 = ev[3][0];
			vector3d x2 = vertices[1 + y*GEOPATCH_EDGELEN];
			vector3d y1 = vertices[(y-1)*GEOPATCH_EDGELEN];
			vector3d y2 = ev[2][GEOPATCH_EDGELEN-1];
			normals[y*GEOPATCH_EDGELEN] = vector3d::Cross(x2-x1, y2-y1).Normalized();
		}

		for (int i=0; i<4; i++) if(!doneEdge[i]) FixEdgeNormals(i, ev[i]);

		UpdateVBOs();
	}

	void GenerateMesh() {
		if (!vertices) {
			vertices = new vector3d[GEOPATCH_NUMVERTICES];
			colors = new vector3f[GEOPATCH_NUMVERTICES];
			vector3d *vts = vertices;
			for (int y=0; y<GEOPATCH_EDGELEN; y++) {
				for (int x=0; x<GEOPATCH_EDGELEN; x++) {
					double height;
					*vts = geosphere->GenPoint(x, y, this, &height);
					vts++;
				}
			}
			assert(vts == &vertices[GEOPATCH_NUMVERTICES]);
		}
			/* XXX some tests to ensure vertices match */
		/*	for (int i=0; i<4; i++) {
				GeoPatch *edge = edgeFriend[i];
				if (edge) {
					int we_are = edge->GetEdgeIdxOf(this);
					assert(v[i] == edge->v[(1+we_are)%4]);
					assert(v[(i+1)%4] == edge->v[(we_are)%4]);
				}
			}*/

	}
	void OnEdgeFriendChanged(int edge, GeoPatch *e) {
		edgeFriend[edge] = e;
		vector3d ev[GEOPATCH_EDGELEN];
		int we_are = e->GetEdgeIdxOf(this);
		e->GetEdgeMinusOneVerticesFlipped(we_are, ev);
		/* now we have a valid edge, fix the edge vertices */
		if (edge == 0) {
			for (int x=0; x<GEOPATCH_EDGELEN; x++) vertices[x] = geosphere->GenPoint(x, 0, this, 0);
		} else if (edge == 1) {
			for (int y=0; y<GEOPATCH_EDGELEN; y++)
				vertices[(GEOPATCH_EDGELEN-1) + GEOPATCH_EDGELEN*y] = geosphere->GenPoint(GEOPATCH_EDGELEN-1, y, this, 0);
		} else if (edge == 2) {
			for (int x=0; x<GEOPATCH_EDGELEN; x++)
				vertices[x + (GEOPATCH_EDGELEN-1)*GEOPATCH_EDGELEN] = geosphere->GenPoint(x, GEOPATCH_EDGELEN-1, this, 0);
		} else {
			for (int y=0; y<GEOPATCH_EDGELEN; y++)
				vertices[y*GEOPATCH_EDGELEN] = geosphere->GenPoint(0, y, this, 0);
		}

		FixEdgeNormals(edge, ev);
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
			glShadeModel(GL_SMOOTH);
			glEnableClientState(GL_VERTEX_ARRAY);
			glEnableClientState(GL_NORMAL_ARRAY);
		//	glEnableClientState(GL_COLOR_ARRAY);
#ifdef USE_VBO
			glBindBufferARB(GL_ARRAY_BUFFER, m_vbo[0]);
			glVertexPointer(3, GL_DOUBLE, 0, 0);
			glBindBufferARB(GL_ARRAY_BUFFER, m_vbo[1]);
			glNormalPointer(GL_DOUBLE, 0, 0);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, indices_vbo);
			glEnableClientState(GL_ELEMENT_ARRAY_BUFFER);
			glDrawElements(GL_TRIANGLES, 2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1)*3, GL_UNSIGNED_INT, 0);
			glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
#else
			glVertexPointer(3, GL_DOUBLE, 0, &vertices[0].x);
			glNormalPointer(GL_DOUBLE, 0, &normals[0].x);
		//	glColorPointer(3, GL_FLOAT, 0, &colors[0].x);
			glDrawElements(GL_TRIANGLES, 2*(GEOPATCH_EDGELEN-1)*(GEOPATCH_EDGELEN-1)*3, GL_UNSIGNED_SHORT, indices);
#endif /* USE_VBO */
			glDisableClientState(GL_VERTEX_ARRAY);
			glDisableClientState(GL_NORMAL_ARRAY);
			glDisableClientState(GL_ELEMENT_ARRAY_BUFFER);
		}
	}

	void LODUpdate(vector3d &campos) {
				
		vector3d centroid = (v[0]+v[1]+v[2]+v[3])*0.25;

		bool canSplit = true;
		for (int i=0; i<4; i++) {
			if (!edgeFriend[i]) { canSplit = false; break; }
			if (edgeFriend[i] && (edgeFriend[i]->m_depth < m_depth)) {
				canSplit = false;
				break;
			}
		}
		if (!(canSplit && (m_depth < GEOPATCH_MAX_DEPTH) &&
		    ((campos - centroid).Length() < m_roughLength*GEOPATCH_SUBDIVIDE_AT_CAMDIST)))
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
				kids[0] = new GeoPatch(v[0], v01, centroid, v30);
				kids[1] = new GeoPatch(v01, v[1], v12, centroid);
				kids[2] = new GeoPatch(centroid, v12, v[2], v23);
				kids[3] = new GeoPatch(v30, centroid, v23, v[3]);
				kids[0]->m_depth = m_depth+1;
				kids[1]->m_depth = m_depth+1;
				kids[2]->m_depth = m_depth+1;
				kids[3]->m_depth = m_depth+1;
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
GLuint GeoPatch::indices_vbo = 0;

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

GeoSphere::GeoSphere()
{
	m_numCraters = 0;
	m_craters = 0;
	float col[4] = {1,1,1,1};
	SetColor(col);
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

	m_patches[0] = new GeoPatch(p1, p2, p3, p4);
	m_patches[1] = new GeoPatch(p4, p3, p7, p8);
	m_patches[2] = new GeoPatch(p1, p4, p8, p5);
	m_patches[3] = new GeoPatch(p2, p1, p5, p6);
	m_patches[4] = new GeoPatch(p3, p2, p6, p7);
	m_patches[5] = new GeoPatch(p8, p7, p6, p5);
	for (int i=0; i<6; i++) {
		m_patches[i]->geosphere = this;
		for (int j=0; j<4; j++) {
			m_patches[i]->edgeFriend[j] = m_patches[geo_sphere_edge_friends[i][j]];
		}
	}
	for (int i=0; i<6; i++) m_patches[i]->GenerateMesh();
	for (int i=0; i<6; i++) m_patches[i]->GenerateNormals();
}

GeoSphere::~GeoSphere()
{
	for (int i=0; i<6; i++) delete m_patches[i];
	delete [] m_craters;
}

void GeoSphere::Render(vector3d campos) {
	for (int i=0; i<6; i++) {
		m_patches[i]->LODUpdate(campos);
	}
	glMaterialfv (GL_FRONT, GL_AMBIENT, m_ambColor);
	glMaterialfv (GL_FRONT, GL_DIFFUSE, m_diffColor);
//	glEnable(GL_COLOR_MATERIAL);
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
double GeoSphere::GetHeight(const vector3d &p)
{
	double n = octavenoise(12, 0.5, 64.0*p);
	double crater = 0;
	for (int i=0; i<m_numCraters; i++) {
		const double dot = vector3d::Dot(m_craters[i].pos, p);
		const double depth = 2.0*(m_craters[i].size - 1.0);
		if (dot > m_craters[i].size) crater += depth * MIN(-(m_craters[i].size-dot)/(1.0-m_craters[i].size),
							0.5); // <-- proportion of crater to be slope, before flat bottom
	}
	return 0.001*n + crater;
}

inline vector3d GeoSphere::GenPoint(const int x, const int y, const GeoPatch *gp, double *height) {
	double xpos = x/(double)(GEOPATCH_EDGELEN-1);
	double ypos = y/(double)(GEOPATCH_EDGELEN-1);
	vector3d p = gp->v[0] + xpos*(1.0-ypos)*(gp->v[1]-gp->v[0]) +
			    xpos*ypos*(gp->v[2]-gp->v[0]) +
			    (1.0-xpos)*ypos*(gp->v[3]-gp->v[0]);
	p = p.Normalized();

	double h = GetHeight(p);
	if (height) *height = h;
	return p + p*h;
}

