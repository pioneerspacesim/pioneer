// Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _GEOSPHERE_H
#define _GEOSPHERE_H

#include <SDL_stdinc.h>

#include "vector3.h"
#include "Random.h"
#include "Camera.h"
#include "galaxy/StarSystem.h"
#include "graphics/Material.h"
#include "terrain/Terrain.h"

namespace Graphics { class Renderer; }
class SystemBody;
class GeoPatch;
class GeoPatchContext;
class GeoSphere {
public:
	GeoSphere(const SystemBody *body);
	~GeoSphere();
	void Render(Graphics::Renderer *renderer, vector3d campos, const float radius, const float scale, const std::vector<Camera::Shadow> &shadows);
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

	struct MaterialParameters {
		SystemBody::AtmosphereParameters atmosphere;
		std::vector<Camera::Shadow> shadows;
	};

private:
	void BuildFirstPatches();
	GeoPatch *m_patches[6];
	const SystemBody *m_sbody;

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

	void SetUpMaterials();
	ScopedPtr<Graphics::Material> m_surfaceMaterial;
	ScopedPtr<Graphics::Material> m_atmosphereMaterial;
	//special parameters for shaders
	MaterialParameters m_materialParameters;
};

#endif /* _GEOSPHERE_H */
