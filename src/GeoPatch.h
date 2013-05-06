// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOPATCH_H
#define _GEOPATCH_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "galaxy/StarSystem.h"
#include "graphics/Material.h"
#include "terrain/Terrain.h"
#include "GeoPatchID.h"
#include "JobQueue.h"

#include <deque>

namespace Graphics { class Renderer; class Frustum; }
class SystemBody;
class GeoPatch;
class GeoPatchContext;
class GeoSphere;
class BasePatchJob;
class SQuadSplitResult;
class SSingleSplitResult;

class GeoPatch {
public:
	static const int NUM_EDGES = 4;
	static const int NUM_KIDS = NUM_EDGES;

	RefCountedPtr<GeoPatchContext> ctx;
	const vector3d v0, v1, v2, v3;
	ScopedArray<double> heights;
	ScopedArray<vector3f> normals;
	ScopedArray<Color3ub> colors;
	GLuint m_vbo;
	ScopedPtr<GeoPatch> kids[NUM_KIDS];
	GeoPatch *parent;
	GeoPatch *edgeFriend[NUM_EDGES]; // [0]=v01, [1]=v12, [2]=v20
	GeoSphere *geosphere;
	double m_roughLength;
	vector3d clipCentroid, centroid;
	double clipRadius;
	int m_depth;
	bool m_needUpdateVBOs;

	const GeoPatchID mPatchID;
	bool mHasJobRequest;

	GeoPatch(const RefCountedPtr<GeoPatchContext> &_ctx, GeoSphere *gs, 
		const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, 
		const int depth, const GeoPatchID &ID_);

	~GeoPatch();

	inline void UpdateVBOs() {
		m_needUpdateVBOs = (NULL!=heights);
	}

	void _UpdateVBOs();

	inline int GetEdgeIdxOf(const GeoPatch *e) const {
		for (int i=0; i<NUM_KIDS; i++) {if (edgeFriend[i] == e) {return i;}}
		abort();
		return -1;
	}

	int GetChildIdx(const GeoPatch *child) const {
		for (int i=0; i<NUM_KIDS; i++) {
			if (kids[i] == child) return i;
		}
		abort();
		return -1;
	}

	// in patch surface coords, [0,1] 
	inline vector3d GetSpherePoint(const double x, const double y) const {
		return (v0 + x*(1.0-y)*(v1-v0) + x*y*(v2-v0) + (1.0-x)*y*(v3-v0)).Normalized();
	}

	inline void OnEdgeFriendChanged(const int edge, GeoPatch *e) {
		edgeFriend[edge] = e;
	}

	inline void NotifyEdgeFriendSplit(GeoPatch *e) {
		if (!kids[0]) {return;}
		const int idx = GetEdgeIdxOf(e);
		const int we_are = e->GetEdgeIdxOf(this);
		// match e's new kids to our own... :/
		kids[idx]->OnEdgeFriendChanged(idx, e->kids[(we_are+1)%NUM_KIDS].Get());
		kids[(idx+1)%NUM_KIDS]->OnEdgeFriendChanged(idx, e->kids[we_are].Get());
	}

	void NotifyEdgeFriendDeleted(const GeoPatch *e) {
		const int idx = GetEdgeIdxOf(e);
		assert(idx>=0 && idx<NUM_EDGES);
		edgeFriend[idx] = NULL;
	}

	inline GeoPatch *GetEdgeFriendForKid(const int kid, const int edge) const {
		const GeoPatch *e = edgeFriend[edge];
		if (!e) return NULL;
		//assert (e);// && (e->m_depth >= m_depth));
		const int we_are = e->GetEdgeIdxOf(this);
		// neighbour patch has not split yet (is at depth of this patch), so kids of this patch do
		// not have same detail level neighbours yet
		if (edge == kid) return e->kids[(we_are+1)%NUM_KIDS].Get();
		else return e->kids[we_are].Get();
	}

	inline GLuint determineIndexbuffer() const {
		return // index buffers are ordered by edge resolution flags
			(edgeFriend[0] ? 1u : 0u) |
			(edgeFriend[1] ? 2u : 0u) |
			(edgeFriend[2] ? 4u : 0u) |
			(edgeFriend[3] ? 8u : 0u);
	}

	void Render(vector3d &campos, const Graphics::Frustum &frustum);

	inline bool canBeMerged() const {
		bool merge = true;
		if (kids[0].Valid()) {
			for (int i=0; i<NUM_KIDS; i++) {
				merge &= kids[i]->canBeMerged();
			}
		}
		merge &= !(mHasJobRequest);
		return merge;
	}

	void LODUpdate(const vector3d &campos);
	
	void RequestSinglePatch();
	void ReceiveHeightmaps(const SQuadSplitResult *psr);
	void ReceiveHeightmap(const SSingleSplitResult *psr);
};

#endif /* _GEOPATCH_H */
