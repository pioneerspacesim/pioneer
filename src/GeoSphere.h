// Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOSPHERE_H
#define _GEOSPHERE_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "Camera.h"
#include "galaxy/StarSystem.h"
#include "graphics/RenderState.h"
#include "graphics/Material.h"
#include "terrain/Terrain.h"
#include "GeoPatchID.h"
#include "BaseSphere.h"

#include <deque>

namespace Graphics { class Renderer; }
namespace CloudJobs { 
	class CloudCPUGenResult;
	class CloudGPUGenResult;
}
class SystemBody;
class GeoPatch;
class GeoPatchContext;
class SQuadSplitRequest;
class SQuadSplitResult;
class SSingleSplitResult;

#define NUM_PATCHES 6

class GeoSphere : public BaseSphere {
public:
	GeoSphere(const SystemBody *body);
	virtual ~GeoSphere();

	virtual void Update() override final;
	virtual void Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const std::vector<Camera::Shadow> &shadows) override final;

	virtual double GetHeight(const vector3d &p) const override final {
		const double h = m_terrain->GetHeight(p);
#ifdef DEBUG
		// XXX don't remove this. Fix your fractals instead
		// Fractals absolutely MUST return heights >= 0.0 (one planet radius)
		// otherwise atmosphere and other things break.
		if (h < 0.0) {
			Output("GetHeight({ %f, %f, %f }) returned %f\n", p.x, p.y, p.z, h);
			m_terrain->DebugDump();
			assert(h >= 0.0);
		}
#endif /* DEBUG */
		return h;
	}

	static void Init();
	static void Uninit();
	static void UpdateAllGeoSpheres();
	static void OnChangeDetailLevel();
	static bool OnAddQuadSplitResult(const SystemPath &path, SQuadSplitResult *res);
	static bool OnAddSingleSplitResult(const SystemPath &path, SSingleSplitResult *res);
	static bool OnAddCPUGenResult(const SystemPath &path, CloudJobs::CloudCPUGenResult *res);
	static bool OnAddGPUGenResult(const SystemPath &path, CloudJobs::CloudGPUGenResult *res);

	// in sbody radii
	virtual double GetMaxFeatureHeight() const override final { return m_terrain->GetMaxHeight(); }

	bool AddQuadSplitResult(SQuadSplitResult *res);
	bool AddSingleSplitResult(SSingleSplitResult *res);
	void ProcessSplitResults();

	virtual void Reset() override final;

	inline Sint32 GetMaxDepth() const { return m_maxDepth; }

	void AddQuadSplitRequest(double, SQuadSplitRequest*, GeoPatch*);

private:
	virtual void SetUpMaterials() override final;
	void BuildFirstPatches();
	void CalculateMaxPatchDepth();
	inline vector3d GetColor(const vector3d &p, double height, const vector3d &norm) const {
		return m_terrain->GetColor(p, height, norm);
	}
	void ProcessQuadSplitRequests();

	bool AddCPUGenResult(CloudJobs::CloudCPUGenResult *res);
	bool AddGPUGenResult(CloudJobs::CloudGPUGenResult *res);
	void RequestCloudSphereTexture();
	bool ProcessCloudResults();
	bool ProcessCPUCloudResults();
	bool ProcessGPUCloudResults();

	std::unique_ptr<GeoPatch> m_patches[NUM_PATCHES];
	struct TDistanceRequest {
		TDistanceRequest(double dist, SQuadSplitRequest *pRequest, GeoPatch *pRequester) :
			mDistance(dist), mpRequest(pRequest), mpRequester(pRequester) {}
		double mDistance;
		SQuadSplitRequest *mpRequest;
		GeoPatch *mpRequester;
	};
	std::deque<TDistanceRequest> mQuadSplitRequests;

	static const uint32_t MAX_SPLIT_OPERATIONS = 128;
	std::deque<SQuadSplitResult*> mQuadSplitResults;
	std::deque<SSingleSplitResult*> mSingleSplitResults;
	std::deque<CloudJobs::CloudCPUGenResult*> mCPUCloudResults;
	std::deque<CloudJobs::CloudGPUGenResult*> mGPUCloudResults;
	Sint32 mCloudProcessDelay;

	bool m_hasTempCampos;
	vector3d m_tempCampos;
	Graphics::Frustum m_tempFrustum;

	static RefCountedPtr<GeoPatchContext> s_patchContext;

	RefCountedPtr<Graphics::Texture> m_texHi;
	RefCountedPtr<Graphics::Texture> m_texLo;
	RefCountedPtr<Graphics::Texture> m_cloudsTexture;
	RefCountedPtr<Graphics::Texture> m_builtTexture;
	
	std::unique_ptr<Color[]> m_jobColorBuffers[NUM_PATCHES];
	Job::Handle m_job[NUM_PATCHES];
	bool m_hasJobRequest[NUM_PATCHES];

	Job::Handle m_gpuJob;
	bool m_hasGpuJobRequest;

	enum EGSInitialisationStage {
		eBuildFirstPatches=0,
		eRequestedFirstPatches,
		eReceivedFirstPatches,
		eDefaultUpdateState
	};
	EGSInitialisationStage m_initStage;

	Sint32 m_maxDepth;
};

#endif /* _GEOSPHERE_H */
