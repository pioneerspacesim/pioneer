#ifndef _GEOSPHERE_H
#define _GEOSPHERE_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "mtrand.h"
#include "GeoSphereStyle.h"

extern int GEOPATCH_EDGELEN;
#define ATMOSPHERE_RADIUS 1.015

class SBody;
class GeoPatch;
class GeoPatchContext;
class GeoSphere {
public:
	GeoSphere(const SBody *body);
	~GeoSphere();
	void Render(vector3d campos, const float radius, const float scale);
	inline double GetHeight(vector3d p) {
		const double h = m_style.GetHeight(p);
		s_vtxGenCount++;
#ifdef DEBUG
		// XXX don't remove this. Fix your fractals instead
		// Fractals absolutely MUST return heights >= 0.0 (one planet radius)
		// otherwise atmosphere and other things break.
		assert(h >= 0.0);
#endif /* DEBUG */
		return h;
	}
	// only called from fishy thread
	void _UpdateLODs();
	friend class GeoPatch;
#if OBJECTVIEWER
	friend class ObjectViewerView;
#endif /* DEBUG */
	static void Init();
	static void OnChangeDetailLevel();
	void GetAtmosphereFlavor(Color *outColor, double *outDensity) const {
		m_style.GetAtmosphereFlavor(outColor, outDensity);
	}
	// in sbody radii
	double GetMaxFeatureHeight() const { return m_style.GetMaxHeight(); }
	static int GetVtxGenCount() { return s_vtxGenCount; }
	static void ClearVtxGenCount() { s_vtxGenCount = 0; }
private:
	void BuildFirstPatches();
	GeoPatch *m_patches[6];
	float m_diffColor[4], m_ambColor[4];
	const SBody *m_sbody;

	/* all variables for GetHeight(), GetColor() */
	GeoSphereStyle m_style;

	///////////////////////////
	// threading rubbbbbish
	// update thread can't do it since only 1 thread can molest opengl
	static int UpdateLODThread(void *data) __attribute((noreturn));
	std::list<GLuint> m_vbosToDestroy;
	SDL_mutex *m_vbosToDestroyLock;
	void AddVBOToDestroy(GLuint vbo);
	void DestroyVBOs();
	
	vector3d m_tempCampos;

	SDL_mutex *m_updateLock;

	SDL_mutex *m_needUpdateLock;
	bool m_needUpdate;

	SDL_mutex *m_abortLock;
	bool m_abort;
	//////////////////////////////

	inline vector3d GetColor(const vector3d &p, double height, const vector3d &norm) {
		return m_style.GetColor(p, height, norm);
	}

	static int s_vtxGenCount;

	static GeoPatchContext *s_patchContext;
};

#endif /* _GEOSPHERE_H */
