// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "GeoPatchContext.h"
#include "GeoPatch.h"
#include "GeoSphere.h"
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

// tri edge lengths
#define GEOPATCH_SUBDIVIDE_AT_CAMDIST	5.0
#define GEOPATCH_MAX_DEPTH  15 + (2*Pi::detail.fracmult) //15

static const int GEOPATCH_MAX_EDGELEN = 55;

void GeoPatch::PatchJob::job_onFinish(void * userData, int userId)  // runs in primary thread of the context
{
	mData->pGeoSphere->AddSplitResult(mpResults);
	PureJob::job_onFinish(userData, userId);
}

void GeoPatch::PatchJob::job_process(void * userData,int /* userId */)    // RUNS IN ANOTHER THREAD!! MUST BE THREAD SAFE!
{
	const SSplitRequestDescription &srd = (*mData.Get());
	const vector3d v01	= (srd.v0+srd.v1).Normalized();
	const vector3d v12	= (srd.v1+srd.v2).Normalized();
	const vector3d v23	= (srd.v2+srd.v3).Normalized();
	const vector3d v30	= (srd.v3+srd.v0).Normalized();
	const vector3d cn	= (srd.centroid).Normalized();

	// 
	const vector3d vecs[4][4] = {
		{srd.v0,	v01,		cn,			v30},
		{v01,		srd.v1,		v12,		cn},
		{cn,		v12,		srd.v2,		v23},
		{v30,		cn,			v23,		srd.v3}
	};

	SSplitResult *sr = new SSplitResult(srd.patchID.GetPatchFaceIdx(), srd.depth);
	for (int i=0; i<4; i++)
	{
		// fill out the data
		GenerateMesh(srd.vertices[i], srd.normals[i], srd.colors[i], 
			vecs[i][0], vecs[i][1], vecs[i][2], vecs[i][3], 
			srd.edgeLen, srd.fracStep);
		// add this patches data
		sr->addResult(srd.vertices[i], srd.normals[i], srd.colors[i], 
			vecs[i][0], vecs[i][1], vecs[i][2], vecs[i][3], 
			srd.patchID.NextPatchID(srd.depth+1, i));
	}
	mpResults = sr;
}

// Generates full-detail vertices, and also non-edge normals and colors 
void GeoPatch::PatchJob::GenerateMesh(
	vector3d *vertices, vector3d *normals, vector3d *colors, 
	const vector3d &v0,
	const vector3d &v1,
	const vector3d &v2,
	const vector3d &v3,
	const int edgeLen,
	const double fracStep) const
{
	const SSplitRequestDescription &srd = (*mData.Get());
	vector3d *vts = vertices;
	vector3d *col = colors;
	double xfrac;
	double yfrac = 0;
	for (int y=0; y<edgeLen; y++) {
		xfrac = 0;
		for (int x=0; x<edgeLen; x++) {
			const vector3d p = GetSpherePoint(v0, v1, v2, v3, xfrac, yfrac);
			const double height = srd.pTerrain->GetHeight(p);
			*(vts++) = p * (height + 1.0);
			// remember this -- we will need it later
			(col++)->x = height;
			xfrac += fracStep;
		}
		yfrac += fracStep;
	}
	assert(vts == &vertices[srd.NUMVERTICES()]);
	// Generate normals & colors for non-edge vertices since they never change
	for (int y=1; y<edgeLen-1; y++) {
		for (int x=1; x<edgeLen-1; x++) {
			// normal
			const vector3d &x1 = vertices[x-1 + y*edgeLen];
			const vector3d &x2 = vertices[x+1 + y*edgeLen];
			const vector3d &y1 = vertices[x + (y-1)*edgeLen];
			const vector3d &y2 = vertices[x + (y+1)*edgeLen];

			const vector3d n = ((x2-x1).Cross(y2-y1)).Normalized();
			normals[x + y*edgeLen] = n;
			// color
			const vector3d p = GetSpherePoint(v0, v1, v2, v3, x*fracStep, y*fracStep);
			const double height = colors[x + y*edgeLen].x;
			vector3d &col_r = colors[x + y*edgeLen];
			col_r = srd.pTerrain->GetColor(p, height, n);
		}
	}
}

GeoPatch::GeoPatch(const RefCountedPtr<GeoPatchContext> &ctx_, GeoSphere *gs, 
	const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, 
	const int depth, const GeoPatchID &ID_) 
	: ctx(ctx_), v0(v0_), v1(v1_), v2(v2_), v3(v3_), 
	vertices(NULL), normals(NULL), colors(NULL), 
	m_vbo(0), parent(NULL), geosphere(gs), 
	m_depth(depth), mPatchID(ID_), 
	mHasSplitRequest(false), mCanMergeChildren(0x0F)
{
	for (int i=0; i<NUM_KIDS; ++i) {
		edgeFriend[i]	= NULL;
		kids[i]			= NULL;
	}
	m_kidsLock = SDL_CreateMutex();
		
	clipCentroid = (v0+v1+v2+v3) * 0.25;
	centroid = clipCentroid.Normalized();
	clipRadius = 0.0;
	clipRadius = std::max(clipRadius, (v0-clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v1-clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v2-clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v3-clipCentroid).Length());
	if (geosphere->m_sbody->type < SystemBody::TYPE_PLANET_ASTEROID) {
 		m_distMult = 10 / Clamp(depth, 1, 10);
 	} else {
 		m_distMult = 5 / Clamp(depth, 1, 5);
 	}
	m_roughLength = GEOPATCH_SUBDIVIDE_AT_CAMDIST / pow(2.0, depth) * m_distMult;
	m_needUpdateVBOs = false;

	geosphere->IncrementCurrentNumPatches();
}

GeoPatch::~GeoPatch() {
	assert(!mHasSplitRequest);

	geosphere->DecrementCurrentNumPatches();
	const int numVerts = ctx->NUMVERTICES();
	const uint64_t memAlloced = (sizeof(vector3d) * numVerts) * 3 * 4;
	geosphere->DelMemAllocatedToPatches(memAlloced);

	SDL_DestroyMutex(m_kidsLock);
	for (int i=0; i<NUM_KIDS; i++) {
		if (edgeFriend[i]) edgeFriend[i]->NotifyEdgeFriendDeleted(this);
	}
	for (int i=0; i<NUM_KIDS; i++) if (kids[i]) delete kids[i];
	delete [] vertices;
	delete [] normals;
	delete [] colors;
	glDeleteBuffersARB(1, &m_vbo);
}

void GeoPatch::_UpdateVBOs() {
	if (m_needUpdateVBOs) {
		m_needUpdateVBOs = false;
		if (!m_vbo) glGenBuffersARB(1, &m_vbo);
		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(GeoPatchContext::VBOVertex)*ctx->NUMVERTICES(), 0, GL_DYNAMIC_DRAW);
		for (int i=0; i<ctx->NUMVERTICES(); i++)
		{
			clipRadius = std::max(clipRadius, (vertices[i]-clipCentroid).Length());
			GeoPatchContext::VBOVertex *pData = ctx->vbotemp + i;
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
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(GeoPatchContext::VBOVertex)*ctx->NUMVERTICES(), ctx->vbotemp, GL_DYNAMIC_DRAW);
		glBindBufferARB(GL_ARRAY_BUFFER, 0);
	}
}

/* not quite edge, since we share edge vertices so that would be
	* fucking pointless. one position inwards. used to make edge normals
	* for adjacent tiles */
void GeoPatch::GetEdgeMinusOneVerticesFlipped(const int edge, vector3d *ev) const {
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

void GeoPatch::FixEdgeNormals(const int edge, const vector3d *ev) {
	int x, y;
	switch (edge) {
	case 0:
		for (x=1; x<ctx->edgeLen-1; x++) {
			const vector3d &x1 = vertices[x-1];
			const vector3d &x2 = vertices[x+1];
			const vector3d &y1 = ev[x];
			const vector3d &y2 = vertices[x + ctx->edgeLen];
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
			const vector3d &x1 = vertices[(x-1) + y*ctx->edgeLen];
			const vector3d &x2 = ev[y];
			const vector3d &y1 = vertices[x + (y-1)*ctx->edgeLen];
			const vector3d &y2 = vertices[x + (y+1)*ctx->edgeLen];
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
			const vector3d &x1 = vertices[x-1 + y*ctx->edgeLen];
			const vector3d &x2 = vertices[x+1 + y*ctx->edgeLen];
			const vector3d &y1 = vertices[x + (y-1)*ctx->edgeLen];
			const vector3d &y2 = ev[ctx->edgeLen-1-x];
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
			const vector3d &x1 = ev[ctx->edgeLen-1-y];
			const vector3d &x2 = vertices[1 + y*ctx->edgeLen];
			const vector3d &y1 = vertices[(y-1)*ctx->edgeLen];
			const vector3d &y2 = vertices[(y+1)*ctx->edgeLen];
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

void GeoPatch::FixEdgeFromParentInterpolated(const int edge) {
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

void GeoPatch::FixCornerNormalsByEdge(const int edge, const vector3d *ev) {
	vector3d ev2[GEOPATCH_MAX_EDGELEN];
	/* XXX All these 'if's have an unfinished else, when a neighbour
		* of our size doesn't exist and instead we must look at a bigger tile.
		* But let's just leave it for the mo because it is a pain.
		* See comment in OnEdgeFriendChanged() */
	switch (edge) {
	case 0:
		if (edgeFriend[3]) {
			const int we_are = edgeFriend[3]->GetEdgeIdxOf(this);
			edgeFriend[3]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
			MakeCornerNormal<0>(ev2, ev);
		}
		if (edgeFriend[1]) {
			const int we_are = edgeFriend[1]->GetEdgeIdxOf(this);
			edgeFriend[1]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
			MakeCornerNormal<1>(ev, ev2);
		}
		break;
	case 1:
		if (edgeFriend[0]) {
			const int we_are = edgeFriend[0]->GetEdgeIdxOf(this);
			edgeFriend[0]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
			MakeCornerNormal<1>(ev2, ev);
		}
		if (edgeFriend[2]) {
			const int we_are = edgeFriend[2]->GetEdgeIdxOf(this);
			edgeFriend[2]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
			MakeCornerNormal<2>(ev, ev2);
		}
		break;
	case 2:
		if (edgeFriend[1]) {
			const int we_are = edgeFriend[1]->GetEdgeIdxOf(this);
			edgeFriend[1]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
			MakeCornerNormal<2>(ev2, ev);
		}
		if (edgeFriend[3]) {
			const int we_are = edgeFriend[3]->GetEdgeIdxOf(this);
			edgeFriend[3]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
			MakeCornerNormal<3>(ev, ev2);
		}
		break;
	case 3:
		if (edgeFriend[2]) {
			const int we_are = edgeFriend[2]->GetEdgeIdxOf(this);
			edgeFriend[2]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
			MakeCornerNormal<3>(ev2, ev);
		}
		if (edgeFriend[0]) {
			const int we_are = edgeFriend[0]->GetEdgeIdxOf(this);
			edgeFriend[0]->GetEdgeMinusOneVerticesFlipped(we_are, ev2);
			MakeCornerNormal<0>(ev, ev2);
		}
		break;
	}

}

void GeoPatch::GenerateEdgeNormalsAndColors() {
	vector3d ev[NUM_EDGES][GEOPATCH_MAX_EDGELEN];
	bool doneEdge[NUM_EDGES];
	memset(doneEdge, 0, sizeof(doneEdge));
	for (int i=0; i<NUM_EDGES; i++) {
		GeoPatch *e = edgeFriend[i];
		if (e) {
			int we_are = e->GetEdgeIdxOf(this);
			e->GetEdgeMinusOneVerticesFlipped(we_are, ev[i]);
		} else if (parent && parent->edgeFriend[i]) {
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

	for (int i=0; i<NUM_EDGES; i++) if(!doneEdge[i]) FixEdgeNormals(i, ev[i]);
}

// Generates full-detail vertices, and also non-edge normals and colors
void GeoPatch::GenerateMesh() {
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
			const vector3d &x1 = vertices[x-1 + y*ctx->edgeLen];
			const vector3d &x2 = vertices[x+1 + y*ctx->edgeLen];
			const vector3d &y1 = vertices[x + (y-1)*ctx->edgeLen];
			const vector3d &y2 = vertices[x + (y+1)*ctx->edgeLen];

			const vector3d n = (x2-x1).Cross(y2-y1);
			normals[x + y*ctx->edgeLen] = n.Normalized();
			// color
			const vector3d p = GetSpherePoint(x*ctx->frac, y*ctx->frac);
			vector3d &col_r = colors[x + y*ctx->edgeLen];
			const double height = col_r.x;
			const vector3d &norm = normals[x + y*ctx->edgeLen];
			col_r = geosphere->GetColor(p, height, norm);
		}
	}
}

void GeoPatch::OnEdgeFriendChanged(const int edge, GeoPatch *e) {
	edgeFriend[edge] = e;
	vector3d ev[GEOPATCH_MAX_EDGELEN];
	const int we_are = e->GetEdgeIdxOf(this);
	e->GetEdgeMinusOneVerticesFlipped(we_are, ev);
	/* now we have a valid edge, fix the edge vertices */
	if (edge == 0) {
		for (int x=0; x<ctx->edgeLen; x++) {
			const vector3d p = GetSpherePoint(x * ctx->frac, 0);
			const double height = geosphere->GetHeight(p);
			vertices[x] = p * (height + 1.0);
			// XXX These bounds checks in each edge case are
			// only necessary while the "All these 'if's"
			// comment in FixCornerNormalsByEdge stands
			if ((x>0) && (x<ctx->edgeLen-1)) {
				colors[x].x = height;
			}
		}
	} else if (edge == 1) {
		for (int y=0; y<ctx->edgeLen; y++) {
			const vector3d p = GetSpherePoint(1.0, y * ctx->frac);
			const double height = geosphere->GetHeight(p);
			const int pos = (ctx->edgeLen-1) + y*ctx->edgeLen;
			vertices[pos] = p * (height + 1.0);
			if ((y>0) && (y<ctx->edgeLen-1)) {
				colors[pos].x = height;
			}
		}
	} else if (edge == 2) {
		for (int x=0; x<ctx->edgeLen; x++) {
			const vector3d p = GetSpherePoint(x * ctx->frac, 1.0);
			const double height = geosphere->GetHeight(p);
			const int pos = x + (ctx->edgeLen-1)*ctx->edgeLen;
			vertices[pos] = p * (height + 1.0);
			if ((x>0) && (x<ctx->edgeLen-1)) {
				colors[pos].x = height;
			}
		}
	} else {
		for (int y=0; y<ctx->edgeLen; y++) {
			const vector3d p = GetSpherePoint(0, y * ctx->frac);
			const double height = geosphere->GetHeight(p);
			const int pos = y * ctx->edgeLen;
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

void GeoPatch::NotifyEdgeFriendSplit(GeoPatch *e) {
	if (!kids[0]) {return;}
	const int idx = GetEdgeIdxOf(e);
	const int we_are = e->GetEdgeIdxOf(this);
	// match e's new kids to our own... :/
	kids[idx]->OnEdgeFriendChanged(idx, e->kids[(we_are+1)%NUM_KIDS]);
	kids[(idx+1)%NUM_KIDS]->OnEdgeFriendChanged(idx, e->kids[we_are]);
}

void GeoPatch::NotifyEdgeFriendDeleted(const GeoPatch *e) {
	const int idx = GetEdgeIdxOf(e);
	assert(idx>=0 && idx<NUM_EDGES);
	edgeFriend[idx] = NULL;
	if (!parent) return;
	if (parent->edgeFriend[idx]) {
		FixEdgeFromParentInterpolated(idx);
		UpdateVBOs();
	} else {
		// XXX TODO XXX
		// Bad. not fixing up edges in this case!!!
	}
}

GeoPatch *GeoPatch::GetEdgeFriendForKid(const int kid, const int edge) const {
	const GeoPatch *e = edgeFriend[edge];
	if (!e) return NULL;
	//assert (e);// && (e->m_depth >= m_depth));
	const int we_are = e->GetEdgeIdxOf(this);
	// neighbour patch has not split yet (is at depth of this patch), so kids of this patch do
	// not have same detail level neighbours yet
	if (edge == kid) return e->kids[(we_are+1)%NUM_KIDS];
	else return e->kids[we_are];
}

void GeoPatch::Render(vector3d &campos, const Graphics::Frustum &frustum) {
	PiVerify(SDL_mutexP(m_kidsLock)==0);
	if (kids[0]) {
		for (int i=0; i<NUM_KIDS; i++) kids[i]->Render(campos, frustum);
		SDL_mutexV(m_kidsLock);
	} else if (NULL!=vertices) {
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

		// update the indices used for rendering
		ctx->updateIndexBufferId(determineIndexbuffer());

		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
		glVertexPointer(3, GL_FLOAT, sizeof(GeoPatchContext::VBOVertex), 0);
		glNormalPointer(GL_FLOAT, sizeof(GeoPatchContext::VBOVertex), reinterpret_cast<void *>(3*sizeof(float)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(GeoPatchContext::VBOVertex), reinterpret_cast<void *>(6*sizeof(float)));
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, ctx->indices_vbo);
		glDrawElements(GL_TRIANGLES, ctx->indices_tri_count*3, GL_UNSIGNED_SHORT, 0);
		glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);

		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glPopMatrix();
	}
}

void GeoPatch::LODUpdate(const vector3d &campos) {
	// there should be no LODUpdate'ing when we have active split requests
	if(mHasSplitRequest)
		return;

	bool canSplit = true;
	bool canMerge = (NULL!=kids[0]) && (0==mCanMergeChildren);
	if (canMerge) {
		for (int i=0; i<NUM_KIDS; i++) { 
			canMerge &= kids[i]->canBeMerged();		
		}
	}

	// always split at first level
	if (parent) {
		for (int i=0; i<NUM_EDGES; i++) {
			if (!edgeFriend[i]) { 
				canSplit = false; 
				break; 
			} else if (edgeFriend[i]->m_depth < m_depth) {
				canSplit = false;
				break;
			}
		}
		const float centroidDist = (campos - centroid).Length();
		const bool errorSplit = (centroidDist < m_roughLength);
		if( !(canSplit && (m_depth < GEOPATCH_MAX_DEPTH) && errorSplit) ) 
		{
			canSplit = false;
		}
	}

	if (canSplit) {
		if (!kids[0]) {
			// don't do anything if we can't handle anymore jobs
			if( !Pi::jobs.canAddJob() ) {
				return;
			}

			mHasSplitRequest = true;
			if(parent) {
				// set the bit flag preventing merging
				parent->mCanMergeChildren |= 1<<mPatchID.GetPatchIdx(m_depth);
			}

			const int numVerts = ctx->NUMVERTICES();
			const uint64_t memAlloced = (sizeof(vector3d) * numVerts) * 3 * 4;
			geosphere->AddMemAllocatedToPatches(memAlloced);

			SSplitRequestDescription *ssrd = new SSplitRequestDescription(v0, v1, v2, v3, centroid.Normalized(), m_depth,
						geosphere->m_sbody->path, mPatchID, ctx->edgeLen,
						ctx->frac, geosphere->m_terrain, geosphere);
			assert(!mCurrentJob.Valid());
			mCurrentJob.Reset(new PatchJob(ssrd));
			Pi::jobs.addJob(mCurrentJob.Get(), NULL);
		} else {
			for (int i=0; i<NUM_KIDS; i++) {
				kids[i]->LODUpdate(campos);
			}
		}
	} else if (canMerge) {
		PiVerify(SDL_mutexP(m_kidsLock)==0);
		for (int i=0; i<NUM_KIDS; i++) { 
			delete kids[i]; 
			kids[i] = NULL; 
		}
		PiVerify(SDL_mutexV(m_kidsLock)!=-1);
	}
}

void GeoPatch::ReceiveHeightmaps(const SSplitResult *psr)
{
	if (m_depth<psr->depth) {
		// this should work because each depth should have a common history
		const uint32_t kidIdx = psr->data[0].patchID.GetPatchIdx(m_depth+1);
		kids[kidIdx]->ReceiveHeightmaps(psr);
	} else {
		const int nD = m_depth+1;
		for (int i=0; i<NUM_KIDS; i++)
		{
			assert(NULL==kids[i]);
			kids[i] = new GeoPatch(ctx, geosphere, 
				psr->data[i].v0, psr->data[i].v1, psr->data[i].v2, psr->data[i].v3, 
				nD, psr->data[i].patchID);
		}

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

		for (int i=0; i<NUM_KIDS; i++)
		{
			kids[i]->vertices = psr->data[i].vertices;
			kids[i]->normals = psr->data[i].normals;
			kids[i]->colors = psr->data[i].colors;
		}
		PiVerify(SDL_mutexP(m_kidsLock)==0);
		for (int i=0; i<NUM_EDGES; i++) { if(edgeFriend[i]) edgeFriend[i]->NotifyEdgeFriendSplit(this); }
		for (int i=0; i<NUM_KIDS; i++) {
			kids[i]->GenerateEdgeNormalsAndColors();
			kids[i]->UpdateVBOs();
		}
		PiVerify(SDL_mutexV(m_kidsLock)!=-1);
		assert(mCurrentJob.Valid());
		mCurrentJob.Reset();
		mHasSplitRequest = false;
		if(parent) {
			// remove the bit flag
			parent->mCanMergeChildren &= ~(1<<mPatchID.GetPatchIdx(m_depth));
		}
	}
}

