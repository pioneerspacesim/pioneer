#ifndef _GEOSPHERE_H
#define _GEOSPHERE_H

#include "vector3.h"
#include "mtrand.h"

class SBody;
class GeoPatch;
class GeoSphere {
public:
	GeoSphere(const SBody *body);
	~GeoSphere();
	void Render(vector3d campos, const float radius, const float scale);
	void AddCraters(MTRand &rand, int num, double minAng, double maxAng);
	double GetHeight(vector3d p);
	// only called from fishy thread
	void _UpdateLODs();
	friend class GeoPatch;
	static void Init();
	void GetAtmosphereFlavor(Color *outColor, float *outDensity) const;
private:
	GeoPatch *m_patches[6];
	struct crater_t {
		vector3d pos;
		double size;
	} *m_craters;
	int m_numCraters;
	float m_diffColor[4], m_ambColor[4];
	const SBody *m_sbody;

	/* all variables for GetHeight(), GetColor() */
	double m_maxHeight;
	double m_invMaxHeight;
	double m_crap[16];
	double m_sealevel;
	double m_sbodyRadius;
	double m_icyness; // ~1.0 = earth (15c)
	vector3d m_fractalOffset;

	/* for sbodies with a heightMap we load this turd
	 * and use it instead of perlin height function */
	Sint16 *m_heightMap;
	int m_heightMapSizeX;
	int m_heightMapSizeY;

	///////////////////////////
	// threading rubbbbbish
	// update thread can't do it since only 1 thread can molest opengl
	static int UpdateLODThread(void *data);
	std::list<GLuint> m_vbosToDestroy;
	SDL_mutex *m_vbosToDestroyLock;
	void AddVBOToDestroy(GLuint vbo);
	void DestroyVBOs();
	
	vector3d m_tempCampos;
	int m_runUpdateThread;
	//////////////////////////////

	int GetRawHeightMapVal(int x, int y);
	double GetHeightMapVal(const vector3d &pt);

	vector3d GetColor(vector3d &p, double height);
};

#endif /* _GEOSPHERE_H */
