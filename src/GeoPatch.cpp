// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "GeoPatchContext.h"
#include "GeoPatch.h"
#include "GeoPatchJobs.h"
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
static const double GEOPATCH_SUBDIVIDE_AT_CAMDIST = 5.0;
#define GEOPATCH_MAX_DEPTH  15 + (2*Pi::detail.fracmult) //15

GeoPatch::GeoPatch(const RefCountedPtr<GeoPatchContext> &ctx_, GeoSphere *gs,
	const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_,
	const int depth, const GeoPatchID &ID_)
	: ctx(ctx_), v0(v0_), v1(v1_), v2(v2_), v3(v3_),
	heights(NULL), normals(NULL), colors(NULL),
	m_vbo(0), parent(NULL), geosphere(gs),
	m_depth(depth), mPatchID(ID_),
	mHasJobRequest(false)
{
	for (int i=0; i<NUM_KIDS; ++i) {
		edgeFriend[i]	= NULL;
	}

	clipCentroid = (v0+v1+v2+v3) * 0.25;
	centroid = clipCentroid.Normalized();
	clipRadius = 0.0;
	clipRadius = std::max(clipRadius, (v0-clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v1-clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v2-clipCentroid).Length());
	clipRadius = std::max(clipRadius, (v3-clipCentroid).Length());
	double distMult;
	if (geosphere->m_sbody->type < SystemBody::TYPE_PLANET_ASTEROID) {
 		distMult = 10.0 / Clamp(depth, 1, 10);
 	} else {
 		distMult = 5.0 / Clamp(depth, 1, 5);
 	}
	m_roughLength = GEOPATCH_SUBDIVIDE_AT_CAMDIST / pow(2.0, depth) * distMult;
	m_needUpdateVBOs = false;
}

GeoPatch::~GeoPatch() {
	mHasJobRequest = false;

	for (int i=0; i<NUM_KIDS; i++) {
		if (edgeFriend[i]) edgeFriend[i]->NotifyEdgeFriendDeleted(this);
	}
	for (int i=0; i<NUM_KIDS; i++) {
		kids[i].Reset();
	}
	heights.Reset();
	normals.Reset();
	colors.Reset();
	glDeleteBuffersARB(1, &m_vbo);
}

void GeoPatch::_UpdateVBOs() {
	if (m_needUpdateVBOs) {
		m_needUpdateVBOs = false;
		if (!m_vbo) glGenBuffersARB(1, &m_vbo);
		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(GeoPatchContext::VBOVertex)*ctx->NUMVERTICES(), 0, GL_DYNAMIC_DRAW);
		double xfrac=0.0, yfrac=0.0;
		double *pHts = heights.Get();
		const vector3f *pNorm = &normals[0];
		const Color3ub *pColr = &colors[0];
		GeoPatchContext::VBOVertex *pData = ctx->vbotemp;
		for (int y=0; y<ctx->edgeLen; y++) {
			xfrac = 0.0;
			for (int x=0; x<ctx->edgeLen; x++) {
				const double height = *pHts;
				const vector3d p = (GetSpherePoint(xfrac, yfrac) * (height + 1.0)) - clipCentroid;
				clipRadius = std::max(clipRadius, p.Length());
				pData->x = float(p.x);
				pData->y = float(p.y);
				pData->z = float(p.z);
				++pHts;	// next height

				pData->nx = pNorm->x;
				pData->ny = pNorm->y;
				pData->nz = pNorm->z;
				++pNorm; // next normal

				pData->col[0] = pColr->r;
				pData->col[1] = pColr->g;
				pData->col[2] = pColr->b;
				pData->col[3] = 255;
				++pColr; // next colour

				++pData; // next vertex

				xfrac += ctx->frac;
			}
			yfrac += ctx->frac;
		}
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(GeoPatchContext::VBOVertex)*ctx->NUMVERTICES(), ctx->vbotemp, GL_DYNAMIC_DRAW);
		glBindBufferARB(GL_ARRAY_BUFFER, 0);
	}
}

void GeoPatch::Render(Graphics::Renderer *renderer, const vector3d &campos, const matrix4x4d &modelView, const Graphics::Frustum &frustum) {
	if (kids[0]) {
		for (int i=0; i<NUM_KIDS; i++) kids[i]->Render(renderer, campos, modelView, frustum);
	} else if (heights.Valid()) {
		_UpdateVBOs();

		if (!frustum.TestPoint(clipCentroid, clipRadius))
			return;

		const vector3d relpos = clipCentroid - campos;
		renderer->SetTransform(modelView * matrix4x4d::Translation(relpos));

		Pi::statSceneTris += 2*(ctx->edgeLen-1)*(ctx->edgeLen-1);

		// update the indices used for rendering
		ctx->updateIndexBufferId(determineIndexbuffer());

		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
		glVertexPointer(3, GL_FLOAT, sizeof(GeoPatchContext::VBOVertex), 0);
		glNormalPointer(GL_FLOAT, sizeof(GeoPatchContext::VBOVertex), reinterpret_cast<void *>(3*sizeof(float)));
		glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(GeoPatchContext::VBOVertex), reinterpret_cast<void *>(6*sizeof(float)));
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, ctx->indices_vbo);
		glDrawElements(GL_TRIANGLES, ctx->indices_tri_count*3, GL_UNSIGNED_SHORT, 0);
	}
}

void GeoPatch::LODUpdate(const vector3d &campos) {
	// there should be no LODUpdate'ing when we have active split requests
	if(mHasJobRequest)
		return;

	bool canSplit = true;
	bool canMerge = kids[0].Valid();

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
		if( !(canSplit && (m_depth < GEOPATCH_MAX_DEPTH) && errorSplit) ) {
			canSplit = false;
		}
	}

	if (canSplit) {
		if (!kids[0]) {
            assert(!mHasJobRequest);
			mHasJobRequest = true;

			SQuadSplitRequest *ssrd = new SQuadSplitRequest(v0, v1, v2, v3, centroid.Normalized(), m_depth,
						geosphere->m_sbody->path, mPatchID, ctx->edgeLen,
						ctx->frac, Terrain::InstanceTerrain(geosphere->m_sbody));
			Pi::Jobs()->Queue(new QuadPatchJob(ssrd));
		} else {
			for (int i=0; i<NUM_KIDS; i++) {
				kids[i]->LODUpdate(campos);
			}
		}
	} else if (canMerge) {
		for (int i=0; i<NUM_KIDS; i++) {
			canMerge &= kids[i]->canBeMerged();
		}
		if( canMerge ) {
			for (int i=0; i<NUM_KIDS; i++) {
				kids[i].Reset();
			}
		}
	}
}

void GeoPatch::RequestSinglePatch()
{
	if( !heights.Valid() ) {
        assert(!mHasJobRequest);
		mHasJobRequest = true;
		SSingleSplitRequest *ssrd = new SSingleSplitRequest(v0, v1, v2, v3, centroid.Normalized(), m_depth,
					geosphere->m_sbody->path, mPatchID, ctx->edgeLen, ctx->frac, Terrain::InstanceTerrain(geosphere->m_sbody));
		Pi::Jobs()->Queue(new SinglePatchJob(ssrd));
	}
}

void GeoPatch::ReceiveHeightmaps(SQuadSplitResult *psr)
{
	assert(NULL!=psr);
	if (m_depth<psr->depth()) {
		// this should work because each depth should have a common history
		const uint32_t kidIdx = psr->data(0).patchID.GetPatchIdx(m_depth+1);
		if( kids[kidIdx] ) {
			kids[kidIdx]->ReceiveHeightmaps(psr);
		} else {
			psr->OnCancel();
		}
	} else {
		assert(mHasJobRequest);
		const int nD = m_depth+1;
		for (int i=0; i<NUM_KIDS; i++)
		{
			assert(!kids[i].Valid());
			const SQuadSplitResult::SSplitResultData& data = psr->data(i);
			assert(i==data.patchID.GetPatchIdx(nD));
			assert(0==data.patchID.GetPatchIdx(nD+1));
			kids[i].Reset(new GeoPatch(ctx, geosphere,
				data.v0, data.v1, data.v2, data.v3,
				nD, data.patchID));
		}

		// hm.. edges. Not right to pass this
		// edgeFriend...
		kids[0]->edgeFriend[0] = GetEdgeFriendForKid(0, 0);
		kids[0]->edgeFriend[1] = kids[1].Get();
		kids[0]->edgeFriend[2] = kids[3].Get();
		kids[0]->edgeFriend[3] = GetEdgeFriendForKid(0, 3);
		kids[1]->edgeFriend[0] = GetEdgeFriendForKid(1, 0);
		kids[1]->edgeFriend[1] = GetEdgeFriendForKid(1, 1);
		kids[1]->edgeFriend[2] = kids[2].Get();
		kids[1]->edgeFriend[3] = kids[0].Get();
		kids[2]->edgeFriend[0] = kids[1].Get();
		kids[2]->edgeFriend[1] = GetEdgeFriendForKid(2, 1);
		kids[2]->edgeFriend[2] = GetEdgeFriendForKid(2, 2);
		kids[2]->edgeFriend[3] = kids[3].Get();
		kids[3]->edgeFriend[0] = kids[0].Get();
		kids[3]->edgeFriend[1] = kids[2].Get();
		kids[3]->edgeFriend[2] = GetEdgeFriendForKid(3, 2);
		kids[3]->edgeFriend[3] = GetEdgeFriendForKid(3, 3);
		kids[0]->parent = kids[1]->parent = kids[2]->parent = kids[3]->parent = this;

		for (int i=0; i<NUM_KIDS; i++)
		{
			const SQuadSplitResult::SSplitResultData& data = psr->data(i);
			kids[i]->heights.Reset(data.heights);
			kids[i]->normals.Reset(data.normals);
			kids[i]->colors.Reset(data.colors);
		}
		for (int i=0; i<NUM_EDGES; i++) { if(edgeFriend[i]) edgeFriend[i]->NotifyEdgeFriendSplit(this); }
		for (int i=0; i<NUM_KIDS; i++) {
			kids[i]->UpdateVBOs();
		}
		mHasJobRequest = false;
	}
}

void GeoPatch::ReceiveHeightmap(const SSingleSplitResult *psr)
{
	assert(NULL==parent);
	assert(NULL!=psr);
	assert(mHasJobRequest);
	{
		const SSingleSplitResult::SSplitResultData& data = psr->data();
		heights.Reset(data.heights);
		normals.Reset(data.normals);
		colors.Reset(data.colors);
	}
	mHasJobRequest = false;
}
