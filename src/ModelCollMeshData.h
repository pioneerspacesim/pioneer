#ifndef _MODELCOLLMESHDATA
#define _MODELCOLLMESHDATA

#include "libs.h"
#include "sbre/sbre.h"

class GeomTree;

class CollMeshSet {
public:
	CollMeshSet(int sbreModel);
	virtual ~CollMeshSet();
	/** returns number of tris found (up to 'num') */
	int GetTrisWithGeomflag(unsigned int flags, int num, vector3d *outVtx) const;
	const Aabb &GetAabb() const;
	int GetNumTris() const { return m_numTris; }

	CollMesh *sbreCollMesh;
	GeomTree *m_geomTree;
private:
	int m_numTris;
};

/* CollMeshSet is cached and caller doesn't own it */
const CollMeshSet *GetModelCollMeshSet(int sbreModel);
const CollMesh *GetModelSBRECollMesh(int sbreModel);

#endif /* _MODELCOLLMESHDATA */
