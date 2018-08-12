// Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
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
class SystemBody;
class GeoPatch;
class GeoPatchContext;
class SQuadSplitRequest;
class SQuadSplitResult;
class SSingleSplitResult;

#define NUM_PATCHES 6

class Regions : public RefCounted
{
public:
	// data about regions from feature to heightmap code go here
	struct RegionType {
		vector3d position;
		double height;
		double inner;
		double outer;
	};
	void SetCityRegions(const std::vector<RegionType> &regions);
	double ApplySimpleHeightRegions(const double h, const vector3d &p) const;

private:
	// used for region based terrain e.g. cities
	std::vector<RegionType> m_regions[3][3][3];
};

class GeoSphere : public BaseSphere {
public:
	GeoSphere(const SystemBody *body);
	virtual ~GeoSphere();

	virtual void Update() override;
	virtual void Render(Graphics::Renderer *renderer, const matrix4x4d &modelView, vector3d campos, const float radius, const std::vector<Camera::Shadow> &shadows) override;

	virtual double GetHeight(const vector3d &p) const override final {
		double h = m_terrain->GetHeight(p);
		h = m_regions->ApplySimpleHeightRegions(h, p);
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
	// in sbody radii
	virtual double GetMaxFeatureHeight() const override final { return m_terrain->GetMaxHeight(); }

	bool AddQuadSplitResult(SQuadSplitResult *res);
	bool AddSingleSplitResult(SSingleSplitResult *res);
	void ProcessSplitResults();

	virtual void Reset() override;

	inline Sint32 GetMaxDepth() const { return m_maxDepth; }
	inline Regions* GetRegions() const { return m_regions.Get(); }

	void AddQuadSplitRequest(double, SQuadSplitRequest*, GeoPatch*);

private:
	void BuildFirstPatches();
	void CalculateMaxPatchDepth();
	inline vector3d GetColor(const vector3d &p, double height, const vector3d &norm) const {
		return m_terrain->GetColor(p, height, norm);
	}
	void ProcessQuadSplitRequests();

	std::unique_ptr<GeoPatch> m_patches[6];
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

	bool m_hasTempCampos;
	vector3d m_tempCampos;
	Graphics::Frustum m_tempFrustum;

	static RefCountedPtr<GeoPatchContext> s_patchContext;

	virtual void SetUpMaterials() override;

	RefCountedPtr<Graphics::Texture> m_texHi;
	RefCountedPtr<Graphics::Texture> m_texLo;

	enum EGSInitialisationStage {
		eBuildFirstPatches=0,
		eRequestedFirstPatches,
		eReceivedFirstPatches,
		eDefaultUpdateState
	};
	EGSInitialisationStage m_initStage;

	Sint32 m_maxDepth;

	RefCountedPtr<Regions> m_regions;
};

#endif /* _GEOSPHERE_H */
