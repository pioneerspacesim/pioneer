#ifndef _MODELCOLLMESHDATA
#define _MODELCOLLMESHDATA

#include "libs.h"
#include "sbre/sbre.h"

struct coltri_t {
	int v1, v2, v3, flags;
};

class CollMeshSet {
public:
	CollMesh *sbreCollMesh;
	coltri_t *triIndices;
	dTriMeshDataID *meshParts;
	int *meshFlags;
	int numMeshParts;

	CollMeshSet(int sbreModel);
private:
	void GetMeshParts();
};

CollMeshSet *GetModelCollMeshSet(int sbreModel);
CollMesh *GetModelSBRECollMesh(int sbreModel);

#endif /* _MODELCOLLMESHDATA */
