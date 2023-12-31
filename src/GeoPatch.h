// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOPATCH_H
#define _GEOPATCH_H

#include <SDL_stdinc.h>

#include "Color.h"
#include "GeoPatchID.h"
#include "JobQueue.h"
#include "RefCounted.h"
#include "matrix4x4.h"
#include "vector3.h"
#include <deque>
#include <memory>

//#define DEBUG_BOUNDING_SPHERES

#ifdef DEBUG_BOUNDING_SPHERES
#include "graphics/Drawables.h"
namespace Graphics {
	class RenderState;
}
#endif

namespace Graphics {
	class Renderer;
	class Frustum;
	class MeshObject;
} // namespace Graphics

class GeoPatchContext;
class GeoSphere;
class BasePatchJob;
class SQuadSplitResult;
class SSingleSplitResult;
struct SSplitResultData;

class GeoPatch {
public:
	GeoPatch(const RefCountedPtr<GeoPatchContext> &_ctx, GeoSphere *gs,
		const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_,
		const int depth, const GeoPatchID &ID_);

	~GeoPatch();

	inline void NeedToUpdateVBOs()
	{
		m_needUpdateVBOs = (nullptr != m_heights);
	}

	void UpdateVBOs(Graphics::Renderer *renderer);

	int GetChildIdx(const GeoPatch *child) const
	{
		for (int i = 0; i < NUM_KIDS; i++) {
			if (m_kids[i].get() == child) return i;
		}
		abort();
		return -1;
	}

	// in patch surface coords, [0,1]
	inline vector3d GetSpherePoint(const double x, const double y) const
	{
		return (m_v0 + x * (1.0 - y) * (m_v1 - m_v0) + x * y * (m_v2 - m_v0) + (1.0 - x) * y * (m_v3 - m_v0)).Normalized();
	}

	void Render(Graphics::Renderer *r, const vector3d &campos, const matrix4x4d &modelView, const Graphics::Frustum &frustum);

	inline bool canBeMerged() const
	{
		bool merge = true;
		if (m_kids[0]) {
			for (int i = 0; i < NUM_KIDS; i++) {
				merge &= m_kids[i]->canBeMerged();
			}
		}
		merge &= !(m_HasJobRequest);
		return merge;
	}

	void LODUpdate(const vector3d &campos, const Graphics::Frustum &frustum);

	void RequestSinglePatch();
	void ReceiveHeightmaps(SQuadSplitResult *psr);
	void ReceiveHeightmap(const SSingleSplitResult *psr);
	void ReceiveHeightResult(const SSplitResultData &data);
	void ReceiveJobHandle(Job::Handle job);

	inline bool HasHeightData() const { return (m_heights.get() != nullptr); }

private:
	static const int NUM_KIDS = 4;

	RefCountedPtr<GeoPatchContext> m_ctx;
	const vector3d m_v0, m_v1, m_v2, m_v3;
	std::unique_ptr<double[]> m_heights;
	std::unique_ptr<vector3f[]> m_normals;
	std::unique_ptr<Color3ub[]> m_colors;
	std::unique_ptr<Graphics::MeshObject> m_patchMesh;
	std::unique_ptr<GeoPatch> m_kids[NUM_KIDS];
	GeoPatch *m_parent;
	GeoSphere *m_geosphere;
	double m_roughLength;
	vector3d m_clipCentroid, m_centroid;
	double m_clipRadius;
	Sint32 m_depth;
	bool m_needUpdateVBOs;

	const GeoPatchID m_PatchID;
	Job::Handle m_job;
	bool m_HasJobRequest;
#ifdef DEBUG_BOUNDING_SPHERES
	std::unique_ptr<Graphics::Drawables::Sphere3D> m_boundsphere;
#endif
};

#endif /* _GEOPATCH_H */
