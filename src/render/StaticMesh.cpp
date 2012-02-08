#include "StaticMesh.h"
#include "Surface.h"

StaticMesh::StaticMesh() :
	numSurfaces(0),
	surfaces(0),
	cached(false),
	m_renderInfo(0)
{
}

StaticMesh::StaticMesh(int n, PrimitiveType t) :
	numSurfaces(0),
	surfaces(0),
	cached(false),
	m_renderInfo(0)
{
	if (n > 0) {
		numSurfaces = n;
		surfaces = new Surface[n];
		for (int i=0;i<n;i++)
			surfaces[i].m_primitiveType = t;
	}
}

StaticMesh::~StaticMesh()
{
	delete m_renderInfo;
	delete[] surfaces;
}

int StaticMesh::GetNumVerts() const
{
	int numvertices = 0;
	for (int i=0; i < numSurfaces; i++) {
		numvertices += surfaces[i].GetNumVerts();
	}
	return numvertices;
}
