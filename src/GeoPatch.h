// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOPATCH_H
#define _GEOPATCH_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "galaxy/StarSystem.h"
#include "graphics/Frustum.h"
#include "graphics/Material.h"
#include "terrain/Terrain.h"
#include "GeoPatchID.h"
#include "JobQueue.h"

#include <deque>

// #define DEBUG_BOUNDING_SPHERES

namespace Graphics { class Renderer; }
class SystemBody;
class GeoPatchContext;
class GeoSphere;
class BasePatchJob;
class SQuadSplitResult;
class SSingleSplitResult;

class GeoPatch {
private:
	static const int NUM_KIDS = 4;

	RefCountedPtr<GeoPatchContext> ctx;
	const vector3d v0, v1, v2, v3;
	std::unique_ptr<double[]> heights;
	std::unique_ptr<vector3f[]> normals;
	std::unique_ptr<Color3ub[]> colors;
	std::unique_ptr<Graphics::VertexBuffer> m_vertexBuffer;
	std::unique_ptr<GeoPatch> kids[NUM_KIDS];
	GeoPatch *parent;
	GeoSphere *geosphere;
	double m_roughLength;
	vector3d clipCentroid, centroid;
	double clipRadius;
	Sint32 m_depth;
	bool m_needUpdateVBOs;

	const GeoPatchID mPatchID;
	Job::Handle m_job;
	bool mHasJobRequest;
#ifdef DEBUG_BOUNDING_SPHERES
	std::unique_ptr<Graphics::Drawables::Sphere3D> m_boundsphere;
#endif
public:

	GeoPatch(const RefCountedPtr<GeoPatchContext> &_ctx, GeoSphere *gs,
		const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_,
		const int depth, const GeoPatchID &ID_);

	~GeoPatch();

	inline void NeedToUpdateVBOs() {
		m_needUpdateVBOs = (nullptr != heights);
	}

	void UpdateVBOs(Graphics::Renderer *renderer);

	int GetChildIdx(const GeoPatch *child) const {
		for (int i=0; i<NUM_KIDS; i++) {
			if (kids[i].get() == child) return i;
		}
		abort();
		return -1;
	}

	// in patch surface coords, [0,1]
	inline vector3d GetSpherePoint(const double x, const double y) const {
		return (v0 + x*(1.0-y)*(v1-v0) + x*y*(v2-v0) + (1.0-x)*y*(v3-v0)).Normalized();
	}

	void Render(Graphics::Renderer *r, const vector3d &campos, const matrix4x4d &modelView, const Graphics::Frustum &frustum);

	inline bool canBeMerged() const {
		bool merge = true;
		if (kids[0]) {
			for (int i=0; i<NUM_KIDS; i++) {
				merge &= kids[i]->canBeMerged();
			}
		}
		merge &= !(mHasJobRequest);
		return merge;
	}

	void LODUpdate(const vector3d &campos, const Graphics::Frustum &frustum);

	void RequestSinglePatch();
	void ReceiveHeightmaps(SQuadSplitResult *psr);
	void ReceiveHeightmap(const SSingleSplitResult *psr);
	void ReceiveJobHandle(Job::Handle job);

	inline bool HasHeightData() const { return (heights.get()!=nullptr); }
};

#endif /* _GEOPATCH_H */
