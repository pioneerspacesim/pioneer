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

struct SSplitRequestDescription {
	SSplitRequestDescription(const vector3d &v0_,
							const vector3d &v1_,
							const vector3d &v2_,
							const vector3d &v3_,
							const vector3d &cn,
							const uint32_t depth_,
							const SystemPath &sysPath_,
							const GeoPatchID &patchID_,
							const int edgeLen_,
							const double fracStep_,
							Terrain *pTerrain_)
							: v0(v0_), v1(v1_), v2(v2_), v3(v3_), centroid(cn), depth(depth_), 
							sysPath(sysPath_), patchID(patchID_), edgeLen(edgeLen_), fracStep(fracStep_), pTerrain(pTerrain_)
	{
	}

	const vector3d v0;
	const vector3d v1;
	const vector3d v2;
	const vector3d v3;
	const vector3d centroid;
	const uint32_t depth;
	const SystemPath sysPath;
	const GeoPatchID patchID;
	const int edgeLen;
	const double fracStep;
	Terrain *pTerrain;
};

struct SSplitResult {
	struct SSplitResultData {
		SSplitResultData(vector3d *v_, vector3d *n_, vector3d *c_, const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const GeoPatchID &patchID_) :
			vertices(v_), normals(n_), colors(c_), v0(v0_), v1(v1_), v2(v2_), v3(v3_), patchID(patchID_)
		{
		}
		vector3d *vertices;
		vector3d *normals;
		vector3d *colors;
		const vector3d v0;
		const vector3d v1;
		const vector3d v2;
		const vector3d v3;
		const GeoPatchID patchID;
	};

	SSplitResult(const int32_t face_, const uint32_t depth_) : face(face_), depth(depth_)
	{
	}

	void addResult(vector3d *v_, vector3d *n_, vector3d *c_, const vector3d &v0_, const vector3d &v1_, const vector3d &v2_, const vector3d &v3_, const GeoPatchID &patchID_)
	{
		data.push_back(SSplitResultData(v_, n_, c_, v0_, v1_, v2_, v3_, patchID_));
		assert(data.size()<=4);
	}

	const int32_t face;
	const uint32_t depth;
	std::deque<SSplitResultData> data;
};

class GeoSphere {
public:
	GeoSphere(const SystemBody *body);
	~GeoSphere();
	void Render(Graphics::Renderer *r, vector3d campos, const float radius, const float scale);
	inline double GetHeight(vector3d p) {
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

	//bool AddSplitRequest(SSplitRequestDescription *desc);
	//void ProcessSplitRequests();
	void ProcessSplitResults();

private:
	void BuildFirstPatches();
	GeoPatch *m_patches[6];
	const SystemBody *m_sbody;

	/* all variables for GetHeight(), GetColor() */
	Terrain *m_terrain;

	static const uint32_t MAX_SPLIT_REQUESTS = 128;
	std::deque<SSplitRequestDescription*> mSplitRequestDescriptions;
	std::deque<SSplitResult*> mSplitResult;

	///////////////////////////
	// threading rubbbbbish
	// update thread can't do it since only 1 thread can molest opengl
	static int UpdateLODThread(void *data);
	std::list<GLuint> m_vbosToDestroy;
	SDL_mutex *m_vbosToDestroyLock;
	void AddVBOToDestroy(GLuint vbo);
	void DestroyVBOs();

	vector3d m_tempCampos;

	SDL_mutex *m_updateLock;
	SDL_mutex *m_abortLock;
	bool m_abort;
	//////////////////////////////

	inline vector3d GetColor(const vector3d &p, double height, const vector3d &norm) {
		return m_terrain->GetColor(p, height, norm);
	}

	static int s_vtxGenCount;

	static RefCountedPtr<GeoPatchContext> s_patchContext;

	void SetUpMaterials();
	ScopedPtr<Graphics::Material> m_surfaceMaterial;
	ScopedPtr<Graphics::Material> m_atmosphereMaterial;
	//special parameters for shaders
	SystemBody::AtmosphereParameters m_atmosphereParameters;
};

#endif /* _GEOSPHERE_H */
