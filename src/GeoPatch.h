// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
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
#include "graphics/Frustum.h"
#include "math/Sphere.h"
#include <deque>
#include <memory>

#define DEBUG_PATCHES 0
//#define DEBUG_BOUNDING_SPHERES

#if DEBUG_PATCHES
#include "graphics/Drawables.h"
namespace Graphics {
#ifdef DEBUG_BOUNDING_SPHERES
	class RenderState;
#endif
	class Label3DWrapper;
} //namespace Graphics
#endif // DEBUG_PATCHES

namespace Graphics {
	class Renderer;
	class MeshObject;
} // namespace Graphics

class GeoPatchContext;
class GeoSphere;
class BasePatchJob;
class SQuadSplitResult;
class SSingleSplitResult;
struct SSplitResultData;

// Experiment:
// Use smaller spheres than the (m_clipCentroid, m_clipRadius) to test against the horizon.
// This can eliminate more patches due to not poking a giant clipping sphere over the horizon
// but on terrains with extreme feature heights there is over-culling of GeoPatches
// eg: Vesta in the Sol system has massive craters which are obviously culled as they go over the horizon.
#define USE_SUB_CENTROID_CLIPPING 1
#if USE_SUB_CENTROID_CLIPPING
#define NUM_HORIZON_POINTS 5 // 9 // 9 points is more expensive
static constexpr double CLIP_RADIUS_MULTIPLIER = 0.1;
#endif // USE_SUB_CENTROID_CLIPPING

class GeoPatch {
public:
	GeoPatch(const RefCountedPtr<GeoPatchContext> &_ctx, GeoSphere *gs,
		const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_,
		const int depth, const GeoPatchID &ID_);

	~GeoPatch();

	void UpdateVBOs(Graphics::Renderer *renderer);

	int GetChildIdx(const GeoPatch *child) const
	{
		for (int i = 0; i < NUM_KIDS; i++) {
			if (m_kids[i].get() == child) return i;
		}
		abort();
		return -1;
	}

	void Render(Graphics::Renderer *renderer, const vector3d &campos, const matrix4x4d &modelView, const Graphics::Frustum &frustum);
	void RenderImmediate(Graphics::Renderer *renderer, const vector3d &campos, const matrix4x4d &modelView) const;
	void GatherRenderablePatches(std::vector<std::pair<double, GeoPatch *>> &visiblePatches, Graphics::Renderer *renderer, const vector3d &campos, const Graphics::Frustum &frustum);

	inline bool canBeMerged() const
	{
		bool merge = true;
		if (m_kids[0]) {
			for (int i = 0; i < NUM_KIDS; i++) {
				merge &= m_kids[i]->canBeMerged();
			}
		}
		merge &= !(m_hasJobRequest);
		return merge;
	}

	void LODUpdate(const vector3d &campos, const Graphics::Frustum &frustum);

	void RequestSinglePatch();
	void ReceiveHeightmaps(SQuadSplitResult *psr);
	void ReceiveHeightmap(const SSingleSplitResult *psr);
	void ReceiveHeightResult(const SSplitResultData &data);
	void ReceiveJobHandle(Job::Handle job);

	inline bool HasHeightData() const { return (m_patchVBOData != nullptr) && (m_patchVBOData->m_heights.get() != nullptr); }

	// used by GeoSphere so must be public
	inline void SetNeedToUpdateVBOs()
	{
		m_needUpdateVBOs = HasHeightData();
	}


private:
	static const int NUM_KIDS = 4;

	bool IsPatchVisible(const Graphics::Frustum &frustum, const vector3d &camPos) const;
	bool IsOverHorizon(const vector3d &camPos) const;

	RefCountedPtr<GeoPatchContext> m_ctx;
	struct Corners {
		Corners(const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_) :
			m_v0(v0_),
			m_v1(v1_),
			m_v2(v2_),
			m_v3(v3_)
		{}
		Corners() = delete;
		const vector3d m_v0, m_v1, m_v2, m_v3;
	}; 

	struct PatchVBOData {
		PatchVBOData() = delete;
		PatchVBOData(double* h, vector3f* n, Color3ub* c)
		{
			m_heights.reset(h);
			m_normals.reset(n);
			m_colors.reset(c);
		}
		~PatchVBOData() {
			m_heights.reset();
			m_normals.reset();
			m_colors.reset();
			m_patchMesh.reset();
		}
		std::unique_ptr<double[]> m_heights;
		std::unique_ptr<vector3f[]> m_normals;
		std::unique_ptr<Color3ub[]> m_colors;
		std::unique_ptr<Graphics::MeshObject> m_patchMesh;
	};

	std::unique_ptr<Corners> m_corners;
	std::unique_ptr<PatchVBOData> m_patchVBOData;
	std::unique_ptr<GeoPatch> m_kids[NUM_KIDS];

	vector3d m_clipCentroid;
#if USE_SUB_CENTROID_CLIPPING
	SSphere m_clipHorizon[NUM_HORIZON_POINTS];
#endif // #if USE_SUB_CENTROID_CLIPPING
	GeoSphere *m_geosphere;
	double m_splitLength; // rough length, is how near to the camera the m_clipCentroid should be before it must split
	double m_clipRadius;
	const GeoPatchID m_PatchID;
	Job::Handle m_job;
	Sint32 m_depth;
	uint8_t m_patchUpdateState;
	bool m_needUpdateVBOs;
	bool m_hasJobRequest;
#if DEBUG_PATCHES
#ifdef DEBUG_BOUNDING_SPHERES
	std::unique_ptr<Graphics::Drawables::Sphere3D> m_boundsphere;
#endif
	std::unique_ptr<Graphics::Drawables::Label3D> m_label3D;

	void RenderLabelDebug(const vector3d &campos, const matrix4x4d &modelView) const;
#endif // #if DEBUG_PATCHES
};

#endif /* _GEOPATCH_H */
