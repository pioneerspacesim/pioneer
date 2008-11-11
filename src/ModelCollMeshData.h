#ifndef _MODELCOLLMESHDATA
#define _MODELCOLLMESHDATA

#include "libs.h"
#include "sbre/sbre.h"

class GeomTree;

struct coltri_t {
	int v1, v2, v3, flags;
};

struct meshinfo_t {
	int flags;
	int triStart; // into triIndices
	int numTris;
};

class CollMeshSet {
public:
	CollMesh *sbreCollMesh;
	coltri_t *triIndices;
	meshinfo_t *meshInfo;
	int numMeshParts;
	Aabb aabb;

	GeomTree *m_geomTree;

	CollMeshSet(int sbreModel);
private:
	void GetMeshParts();
};

CollMeshSet *GetModelCollMeshSet(int sbreModel);
CollMesh *GetModelSBRECollMesh(int sbreModel);

#endif /* _MODELCOLLMESHDATA */
