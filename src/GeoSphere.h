// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOSPHERE_H
#define _GEOSPHERE_H

#include <SDL_stdinc.h>

#include "BaseSphere.h"
#include "Camera.h"
#include "core/Log.h"
#include "vector3.h"

#include <deque>

namespace Graphics {
	class Renderer;
	class Texture;
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

	void Update() override;
	void Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const std::vector<Camera::Shadow> &shadows) override;

	double GetHeight(const vector3d &p) const final
	{
		const double h = m_terrain->GetHeight(p);
#ifndef NDEBUG
		// XXX don't remove this. Fix your fractals instead
		// Fractals absolutely MUST return heights >= 0.0 (one planet radius)
		// otherwise atmosphere and other things break.
		if (h < 0.0) {
			Output("GetHeight({ %f, %f, %f }) returned %f\n", p.x, p.y, p.z, h);
			m_terrain->DebugDump();
			assert(h >= 0.0);
		}
#endif /* NDEBUG */
		return h;
	}

	static void InitGeoSphere();
	static void UninitGeoSphere();
	static void UpdateAllGeoSpheres();
	static void OnChangeGeoSphereDetailLevel();
	static bool OnAddQuadSplitResult(const SystemPath &path, SQuadSplitResult *res);
	static bool OnAddSingleSplitResult(const SystemPath &path, SSingleSplitResult *res);

	enum DebugFlags : uint32_t { // <enum scope='GeoSphere' name=GeoSphereDebugFlags prefix=DEBUG_ public>
		DEBUG_NONE = 0x0,
		DEBUG_SORTGEOPATCHES = 0x1,
		DEBUG_WIREFRAME = 0x2,
		DEBUG_FACELABELS = 0x4
	};
	static void SetDebugFlags(Uint32 flags);
	static Uint32 GetDebugFlags();

	// in sbody radii
	double GetMaxFeatureHeight() const final { return m_terrain->GetMaxHeight(); }

	bool AddQuadSplitResult(SQuadSplitResult *res);
	bool AddSingleSplitResult(SSingleSplitResult *res);
	void ProcessSplitResults();

	void Reset() override;

	inline Sint32 GetMaxDepth() const { return m_maxDepth; }

	void AddQuadSplitRequest(double, SQuadSplitRequest *, GeoPatch *);

private:
	void BuildFirstPatches();
	void CalculateMaxPatchDepth();
	inline vector3d GetColor(const vector3d &p, double height, const vector3d &norm) const
	{
		return m_terrain->GetColor(p, height, norm);
	}
	void ProcessQuadSplitRequests();

	std::unique_ptr<GeoPatch> m_patches[6];
	std::vector<std::pair<double, GeoPatch *>> m_visiblePatches;

	struct TDistanceRequest {
		TDistanceRequest(double dist, SQuadSplitRequest *pRequest, GeoPatch *pRequester) :
			mDistance(dist),
			mpRequest(pRequest),
			mpRequester(pRequester) {}
		double mDistance;
		SQuadSplitRequest *mpRequest;
		GeoPatch *mpRequester;
	};
	std::deque<TDistanceRequest> mQuadSplitRequests;

	static const uint32_t MAX_SPLIT_OPERATIONS = 128;
	std::deque<SQuadSplitResult *> mQuadSplitResults;
	std::deque<SSingleSplitResult *> mSingleSplitResults;

	bool m_hasTempCampos;
	vector3d m_tempCampos;
	Graphics::Frustum m_tempFrustum;

	static RefCountedPtr<GeoPatchContext> s_patchContext;

	void SetUpMaterials() override;
	void CreateAtmosphereMaterial();

	RefCountedPtr<Graphics::Texture> m_texHi;
	RefCountedPtr<Graphics::Texture> m_texLo;

	enum EGSInitialisationStage {
		eBuildFirstPatches = 0,
		eRequestedFirstPatches,
		eReceivedFirstPatches,
		eDefaultUpdateState
	};
	EGSInitialisationStage m_initStage;

	Sint32 m_maxDepth;
};

#endif /* _GEOSPHERE_H */
