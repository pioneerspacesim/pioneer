#include "StaticMesh.h"
#include "Surface.h"

namespace Graphics {

StaticMesh::StaticMesh(PrimitiveType t) :
	Renderable(),
	cached(false),
	m_primitiveType(t)
{
}

StaticMesh::~StaticMesh()
{
	while (!m_surfaces.empty()) delete m_surfaces.back(), m_surfaces.pop_back();
}

void StaticMesh::AddSurface(Surface *s)
{
	m_surfaces.push_back(s);
}

int StaticMesh::GetNumVerts() const
{
	int numvertices = 0;
	for (SurfaceIterator surface = SurfacesBegin(); surface != SurfacesEnd(); ++surface)
		numvertices += (*surface)->GetNumVerts();
	return numvertices;
}

int StaticMesh::GetNumIndices() const
{
	int numIndices = 0;
	for (SurfaceIterator surface = SurfacesBegin(); surface != SurfacesEnd(); ++surface)
		numIndices += (*surface)->GetNumIndices();
	return numIndices;
}

AttributeSet StaticMesh::GetAttributeSet() const
{
	//all vertices should match
	AttributeSet set = 0;
	for (SurfaceIterator surface = SurfacesBegin(); surface != SurfacesEnd(); ++surface)
		if ((*surface)->GetVertices())
			set = (*surface)->GetVertices()->GetAttributeSet();
	return set;
}

}
