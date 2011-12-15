#ifndef _GEOSPHERE_H
#define _GEOSPHERE_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "mtrand.h"
#include "terrain/Terrain.h"

extern int GEOPATCH_EDGELEN;
#define ATMOSPHERE_RADIUS 1.015

struct EclipseData {
	int lightNum;
	vector3d centre;
	double srad;
	double lrad;
};

class SBody;
class GeoPatch;
class GeoPatchContext;
class GeoSphere {
public:
	GeoSphere(const SBody *body);
	~GeoSphere();
	void Render(vector3d campos, const float radius, const float scale);
	inline double GetHeight(vector3d p) {
		const double h = m_terrain->GetHeight(p);
		s_vtxGenCount++;
#ifdef DEBUG
		// XXX don't remove this. Fix your fractals instead
		// Fractals absolutely MUST return heights >= 0.0 (one planet radius)
		// otherwise atmosphere and other things break.
		assert(h >= 0.0);
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

	void AddEclipse(int light, vector3d centre, double srad, double lrad) {
	    const EclipseData eclipse = { light, centre, srad, lrad };
	    m_eclipses.push_back(eclipse);
	}

	void ClearEclipses() { m_eclipses.clear(); }
	void SetLightDiscRadius(int light, float radius)
		{ m_lightDiscRadii[light] = radius; }

private:
	void BuildFirstPatches();
	GeoPatch *m_patches[6];
	float m_diffColor[4], m_ambColor[4];
	const SBody *m_sbody;

	/* all variables for GetHeight(), GetColor() */
	Terrain *m_terrain;

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

	std::list<EclipseData> m_eclipses;
	float m_lightDiscRadii[4];
};

#endif /* _GEOSPHERE_H */
