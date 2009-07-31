#include "libs.h"
#include "ModelCollMeshData.h"
#include <map>
#include <algorithm>
#include "sbre/sbre.h"
#include "collider/collider.h"

// In order to do space station doors and other flagged bits of collision
// meshes, you have to make a different dTriMeshData thing for each.
// vertex indices for each flagged bit of the mesh need to be contiguous,
// so we waste some time and memory making this so.
struct coltri_compare : public std::binary_function<coltri_t, coltri_t, bool> {
	bool operator()(coltri_t a, coltri_t b) { return a.flags < b.flags; }
};

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
	delete [] triIndices;
	delete [] meshInfo;
	delete m_geomTree;
}

void CollMeshSet::GetMeshParts()
{
	numMeshParts = 1;
	int curFlag = triIndices[0].flags;
	for (int i=0; i<sbreCollMesh->ni/3; i++) {
		if (curFlag != triIndices[i].flags) numMeshParts++;
		curFlag = triIndices[i].flags;
	}
//	printf("%d parts\n", numMeshParts);
	meshInfo = new meshinfo_t[numMeshParts];

	int tidx = 0;
	int midx = 0;
	do {
		int len = 0;
		int flag = triIndices[tidx].flags;
		while (((len+tidx) < sbreCollMesh->ni/3) && (triIndices[tidx+len].flags == flag)) len++;
		meshInfo[midx].flags = flag;
		meshInfo[midx].triStart = tidx;
		meshInfo[midx].numTris = len;
		midx++;
		tidx += len;
	} while (tidx < sbreCollMesh->ni/3);
}

CollMeshSet::CollMeshSet(int sbreModel)
{
	aabb.min = vector3d(FLT_MAX, FLT_MAX, FLT_MAX);
	aabb.max = vector3d(-FLT_MIN, -FLT_MIN, -FLT_MIN);
	sbreCollMesh = (CollMesh*)calloc(1, sizeof(CollMesh));
	sbreGenCollMesh(sbreCollMesh, sbreModel, &params);
	// XXX flip Z & X because sbre is in magicspace
	for (int i=0; i<3*sbreCollMesh->nv; i+=3) {
//		sbreCollMesh->pVertex[i] = -sbreCollMesh->pVertex[i];
//		sbreCollMesh->pVertex[i+2] = -sbreCollMesh->pVertex[i+2];
		// making axis aligned bounding box
		for (int a=0; a<3; a++) {
			if (sbreCollMesh->pVertex[i+a] < aabb.min[a])
				aabb.min[a] = sbreCollMesh->pVertex[i+a];
			if (sbreCollMesh->pVertex[i+a] > aabb.max[a])
				aabb.max[a] = sbreCollMesh->pVertex[i+a];
		}
	}
	m_geomTree = new GeomTree(sbreCollMesh->nv, sbreCollMesh->ni/3, sbreCollMesh->pVertex, sbreCollMesh->pIndex, sbreCollMesh->pFlag);

	triIndices = new coltri_t[sbreCollMesh->ni/3];
	int tidx = 0;
	// copy the tri indices into our lovely coltri_t array
	for (int i=0; i<sbreCollMesh->ni; i+=3) {
		triIndices[tidx].v1 = sbreCollMesh->pIndex[i];
		triIndices[tidx].v2 = sbreCollMesh->pIndex[i+1];
		triIndices[tidx].v3 = sbreCollMesh->pIndex[i+2];
		triIndices[tidx].flags = sbreCollMesh->pFlag[i/3];
		tidx++;
	}
	// sort collmesh tris by flag, ascending
	sort(triIndices, triIndices+(sbreCollMesh->ni/3), coltri_compare());

	GetMeshParts();
}

const CollMeshSet *GetModelCollMeshSet(int sbreModel)
{
	std::map<int,CollMeshSet*>::iterator it = modelCollTurds.find(sbreModel);
	if (it != modelCollTurds.end()) return (*it).second;

	CollMeshSet *cturd = new CollMeshSet(sbreModel);
	modelCollTurds[sbreModel] = cturd;
	return cturd;
}

const CollMesh *GetModelSBRECollMesh(int sbreModel)
{
	return modelCollTurds[sbreModel]->sbreCollMesh;
}
