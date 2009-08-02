#include "libs.h"
#include "ModelCollMeshData.h"
#include <map>
#include <algorithm>
#include "sbre/sbre.h"
#include "collider/collider.h"

static ObjParams params = {
	{ 1, 0, 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 1, 0, 0, 0, 0, 1, 1, 0, 0, 0 },
	{ 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.0f },

	{	// pColor[3]
	{ { 1.0f, 0.0f, 1.0f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.8f, 0.6f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 },
	{ { 0.5f, 0.5f, 0.5f }, { 0, 0, 0 }, { 0, 0, 0 }, 0 } },

	// pText[3][256]	
	{ "", "" },
};

// indexed by sbre model ID
static std::map<int, CollMeshSet*> modelCollTurds;

CollMeshSet::~CollMeshSet() {
	// why mix new and malloc/calloc? fucking stupidity that's why
	free(sbreCollMesh);
	delete m_geomTree;
}

const Aabb &CollMeshSet::GetAabb() const
{
	return m_geomTree->GetAabb();
}

CollMeshSet::CollMeshSet(int sbreModel)
{
	sbreCollMesh = (CollMesh*)calloc(1, sizeof(CollMesh));
	sbreGenCollMesh(sbreCollMesh, sbreModel, &params);
	m_geomTree = new GeomTree(sbreCollMesh->nv, sbreCollMesh->ni/3, sbreCollMesh->pVertex, sbreCollMesh->pIndex, sbreCollMesh->pFlag);
	m_numTris = sbreCollMesh->ni/3;
}

const CollMeshSet *GetModelCollMeshSet(int sbreModel)
{
	std::map<int,CollMeshSet*>::iterator it = modelCollTurds.find(sbreModel);
	if (it != modelCollTurds.end()) return (*it).second;

	CollMeshSet *cturd = new CollMeshSet(sbreModel);
	modelCollTurds[sbreModel] = cturd;
	return cturd;
}

/** returns number of tris found (up to 'num') */
int CollMeshSet::GetTrisWithGeomflag(unsigned int flags, int num, vector3d *outVtx) const
{
	int found = 0;
	for (int i=0; (i<m_numTris) && (found<num); i++) {
		if (sbreCollMesh->pFlag[i] == flags) {
			*(outVtx++) = vector3d(&sbreCollMesh->pVertex[3*sbreCollMesh->pIndex[3*i]]);
			*(outVtx++) = vector3d(&sbreCollMesh->pVertex[3*sbreCollMesh->pIndex[3*i+1]]);
			*(outVtx++) = vector3d(&sbreCollMesh->pVertex[3*sbreCollMesh->pIndex[3*i+2]]);
			found++;
		}
	}
	return found;
}

const CollMesh *GetModelSBRECollMesh(int sbreModel)
{
	return modelCollTurds[sbreModel]->sbreCollMesh;
}
