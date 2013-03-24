// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOSPHERE_H
#define _GEOSPHERE_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "galaxy/StarSystem.h"
#include "graphics/Material.h"
#include "terrain/Terrain.h"
#include "GeoPatchID.h"

#include <deque>

namespace Graphics { class Renderer; }
class SystemBody;
class GeoPatch;
class GeoPatchContext;
class GeoSphere;
class SQuadSplitRequest;
class SQuadSplitResult;
class SSingleSplitResult;

#define NUM_PATCHES 6

class GeoSphere {
public:
	GeoSphere(const SystemBody *body);
	~GeoSphere();
	void Update();
	void Render(Graphics::Renderer *r, vector3d campos, const float radius, const float scale);
	inline double GetHeight(vector3d p) const {
		const double h = m_terrain->GetHeight(p);
		s_vtxGenCount++;
#ifdef DEBUG
		// XXX don't remove this. Fix your fractals instead
		// Fractals absolutely MUST return heights >= 0.0 (one planet radius)
		// otherwise atmosphere and other things break.
		if (h < 0.0) {
			fprintf(stderr, "GetHeight({ %f, %f, %f }) returned %f\n", p.x, p.y, p.z, h);
			m_terrain->DebugDump();
			assert(h >= 0.0);
		}
#endif /* DEBUG */
		return h;
	}
	friend class GeoPatch;
	static void Init();
	static void Uninit();
	static void OnChangeDetailLevel();
	// in sbody radii
	double GetMaxFeatureHeight() const { return m_terrain->GetMaxHeight(); }
	static int GetVtxGenCount() { return s_vtxGenCount; }
	static void ClearVtxGenCount() { s_vtxGenCount = 0; }

	//bool AddSplitRequest(SQuadSplitRequest *desc);
	//void ProcessSplitRequests();
	bool AddQuadSplitResult(SQuadSplitResult *res);
	bool AddSingleSplitResult(SSingleSplitResult *res);
	void ProcessSplitResults();

	void Reset();

private:
	void BuildFirstPatches();
	ScopedPtr<GeoPatch> m_patches[6];
	const SystemBody *m_sbody;

	// all variables for GetHeight(), GetColor()
	ScopedPtr<Terrain> m_terrain;

	static const uint32_t MAX_SPLIT_OPERATIONS = 128;
	std::deque<SQuadSplitResult*> mQuadSplitResults;
	std::deque<SSingleSplitResult*> mSingleSplitResults;

	bool m_hasTempCampos;
	vector3d m_tempCampos;

	uint32_t mCurrentNumPatches;
	uint64_t mCurrentMemAllocatedToPatches;

	inline vector3d GetColor(const vector3d &p, double height, const vector3d &norm) const {
		return m_terrain->GetColor(p, height, norm);
	}

	static int s_vtxGenCount;

	static RefCountedPtr<GeoPatchContext> s_patchContext;

	void SetUpMaterials();
	ScopedPtr<Graphics::Material> m_surfaceMaterial;
	ScopedPtr<Graphics::Material> m_atmosphereMaterial;
	//special parameters for shaders
	SystemBody::AtmosphereParameters m_atmosphereParameters;

	enum EGSInitialisationStage {
		eBuildFirstPatches=0,
		eRequestedFirstPatches,
		eReceivedFirstPatches,
		eDefaultUpdateState
	};
	EGSInitialisationStage m_initStage;
};

#endif /* _GEOSPHERE_H */
